#include "wotes_menu.h"
#include "wotes_video.h"
#include "app_config.h"
#include "input_map.h"
#include "gamepad.h"
#include "audio.h"
#include "overlay_draw.h"
#include "host_atomic_file.h"
#include "genesis_runtime.h"
#include "png_write.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

enum { W=960, H=600 };
enum { PAGE_HOME, PAGE_SETTINGS, PAGE_KEYS, PAGE_PAD, PAGE_HELP, PAGE_RESET,
       PAGE_SAVE, PAGE_LOAD, PAGE_REPLACE };
static uint32_t s_pixels[W*H];
static char s_settings[640],s_notice[128];
static Uint32 s_notice_until;
static int s_intro,s_muted;
typedef struct Menu {
    SDL_Renderer *renderer;
    SDL_Window *window;
    int playing,page,selected,capture,axis_x,axis_y,closing,result,renderer_reset,save_slot;
} Menu;

static int clamp(int value,int low,int high) { return value<low?low:value>high?high:value; }
static void notice(const char *text) {
    snprintf(s_notice,sizeof s_notice,"%s",text);s_notice_until=SDL_GetTicks()+3500;
}
static void trace(const char *phase,int detail) {
    const char *path=getenv("WOTES_MENU_TRACE");
    if (!path || !*path) return;
    uint32_t hash=2166136261u;
    for(unsigned i=0;i<sizeof g_ram;++i)hash=(hash^g_ram[i])*16777619u;
    FILE *f=fopen(path,"a");
    if(f){fprintf(f,"%s,%d,%llu,%08x\n",phase,detail,(unsigned long long)g_frame_count,hash);fclose(f);}
}
void wotes_menu_init(const char *settings_path) {
    snprintf(s_settings,sizeof s_settings,"%s",settings_path);
    s_intro=s_muted=0;
    FILE *f=fopen(s_settings,"r");
    if(f) {
        char line[256];int section=0,value;
        while(fgets(line,sizeof line,f)) {
            char *p=line;while(*p==' '||*p=='\t')++p;
            if(*p=='[')section=!strncmp(p,"[wotes_menu]",12);
            if(section && sscanf(p,"intro = %d",&value)==1)s_intro=value!=0;
            if(section && sscanf(p,"mute = %d",&value)==1)s_muted=value!=0;
        }
        fclose(f);
    }
    g_input_map.p[0].device|=INPUT_DEV_KEYBOARD;
}
int wotes_menu_intro_enabled(void) { return s_intro; }
void wotes_menu_state_path(int slot,char *out,size_t cap) {
    const char *a=strrchr(s_settings,'/'),*b=strrchr(s_settings,'\\');
    const char *sep=!a?b:!b?a:a>b?a:b;
    int prefix=sep?(int)(sep-s_settings+1):0;
    if(slot==WOTES_QUICK_SLOT)snprintf(out,cap,"%.*squick_state.bin",prefix,s_settings);
    else snprintf(out,cap,"%.*sstate_%d.bin",prefix,s_settings,slot);
}
static int slot_info(int slot,char *label,size_t cap) {
    char path[800];wotes_menu_state_path(slot,path,sizeof path);
#ifdef _WIN32
    struct _stat64 info;int present=_stat64(path,&info)==0 && info.st_size>0;
#else
    struct stat info;int present=stat(path,&info)==0 && info.st_size>0;
#endif
    if(!present){if(label)snprintf(label,cap,"EMPTY");return 0;}
    if(label) {
        time_t when=(time_t)info.st_mtime;struct tm *local=localtime(&when);
        if(!local || !strftime(label,cap,"%d %b %H:%M",local))snprintf(label,cap,"SAVED");
    }
    return 1;
}
void wotes_menu_apply(void) {
    audio_set_master_volume(s_muted?0:g_app_config.volume);
    app_config_apply_video(&wotes_video);
}
static int save_settings(void) {
    char temporary[800];
    snprintf(temporary,sizeof temporary,"%s.menu-stage.%llu",s_settings,
             (unsigned long long)SDL_GetPerformanceCounter());
    int ok=app_config_save(temporary);
    FILE *f=ok?fopen(temporary,"rb"):NULL;
    char *data=NULL;long bytes=0;
    if(f) {
        if(fseek(f,0,SEEK_END)==0)bytes=ftell(f);
        if(bytes>0 && bytes<65536 && !fseek(f,0,SEEK_SET)) {
            data=malloc((size_t)bytes+128);
            if(data && fread(data,1,(size_t)bytes,f)==(size_t)bytes) {
                int tail=snprintf(data+bytes,128,"\n[wotes_menu]\nintro = %d\nmute = %d\n",s_intro,s_muted);
                ok=genesis_atomic_file_store(s_settings,data,(size_t)bytes+tail);
            } else ok=0;
        } else ok=0;
        fclose(f);
    } else ok=0;
    free(data);remove(temporary);
    notice(ok?"SETTINGS SAVED":"COULD NOT SAVE SETTINGS - CHECK FOLDER ACCESS");
    return ok;
}
void wotes_menu_fullscreen_changed(SDL_Window *window) {
    g_app_config.fullscreen=(SDL_GetWindowFlags(window)&SDL_WINDOW_FULLSCREEN)?1:0;
    save_settings();
}
static void fill(int x,int y,int w,int h,uint32_t color) {
    gen_ovl_fill_rect(s_pixels,W,H,x,y,w,h,color);
}
static void text(int x,int y,const char *s,uint32_t color,int scale) {
    gen_ovl_draw_text(s_pixels,W,H,x,y,s,color,scale);
}
static int count(const Menu *m) {
    switch(m->page) {
        case PAGE_HOME:return m->playing?6:4;
        case PAGE_SAVE:case PAGE_LOAD:return 6;
        case PAGE_SETTINGS:return 9;
        case PAGE_KEYS:return 10;
        case PAGE_PAD:return 12;
        case PAGE_HELP:return 1;
        default:return 2;
    }
}
static int row_y(const Menu *m) {
    return m->page==PAGE_HOME?(m->playing?176:208):(m->page==PAGE_RESET||m->page==PAGE_REPLACE)?322:
           (m->page==PAGE_SAVE||m->page==PAGE_LOAD)?148:138;
}
static int row_h(const Menu *m) {
    return m->page==PAGE_HOME?(m->playing?48:60):(m->page==PAGE_RESET||m->page==PAGE_REPLACE)?54:
           (m->page==PAGE_SAVE||m->page==PAGE_LOAD)?56:m->page==PAGE_PAD?32:38;
}
static void page(Menu *m,int next,int selected) {
    m->page=next;m->selected=selected;m->capture=-1;m->axis_x=m->axis_y=0;
}
static void pad_label(const GamepadBind *bind,char *out,size_t cap) {
    static const char *buttons[]={"SOUTH / A / CROSS","EAST / B / CIRCLE","WEST / X / SQUARE","NORTH / Y / TRIANGLE",
        "BACK / VIEW","GUIDE","START / OPTIONS","LEFT STICK CLICK","RIGHT STICK CLICK","LEFT SHOULDER","RIGHT SHOULDER",
        "D-PAD UP","D-PAD DOWN","D-PAD LEFT","D-PAD RIGHT"};
    if(bind->kind==GP_BIND_BUTTON) {
        if(bind->code>=0 && bind->code<(int)(sizeof buttons/sizeof *buttons))
            snprintf(out,cap,"%s",buttons[bind->code]);
        else snprintf(out,cap,"BUTTON %d",bind->code);
    } else if(bind->kind==GP_BIND_AXIS) {
        static const char *axes[]={"LEFT STICK X","LEFT STICK Y","RIGHT STICK X","RIGHT STICK Y","LEFT TRIGGER","RIGHT TRIGGER"};
        snprintf(out,cap,"%s %s",bind->code>=0 && bind->code<6?axes[bind->code]:"AXIS",bind->axis_dir<0?"-":"+");
    } else snprintf(out,cap,"UNBOUND");
}
static void row_label(const Menu *m,int index,char *label,char *value) {
    label[0]=value[0]=0;
    const PlayerInput *p=&g_input_map.p[0];
    if(m->page==PAGE_HOME) {
        static const char *before[]={"START GAME","LOAD STATE","SETTINGS","EXIT"};
        static const char *during[]={"RESUME","SAVE STATE","LOAD STATE","RESET GAME","SETTINGS","EXIT"};
        snprintf(label,96,"%s",m->playing?during[index]:before[index]);
    } else if(m->page==PAGE_SAVE || m->page==PAGE_LOAD) {
        if(index==5)snprintf(label,96,"BACK");
        else {
            if(index==4)snprintf(label,96,"QUICK SAVE");else snprintf(label,96,"STATE %d",index+1);
            slot_info(index+1,value,96);
        }
    } else if(m->page==PAGE_SETTINGS) {
        static const char *labels[]={"ASPECT RATIO","SHOW GAME INTRO","FULLSCREEN","MASTER VOLUME","MUTE",
            "KEYBOARD CONTROLS","CONTROLLER CONTROLS","CONTROLS GUIDE","BACK"};
        snprintf(label,96,"%s",labels[index]);
        if(index==0)snprintf(value,96,"%s",g_app_config.custom_widescreen?"16:9":"4:3");
        if(index==1)snprintf(value,96,"%s",s_intro?"ON":"OFF");
        if(index==2)snprintf(value,96,"%s",g_app_config.fullscreen?"ON":"OFF");
        if(index==3)snprintf(value,96,"%d%%",g_app_config.volume);
        if(index==4)snprintf(value,96,"%s",s_muted?"ON":"OFF");
    } else if(m->page==PAGE_KEYS) {
        if(index<8) {
            snprintf(label,96,"%s",input_button_name((GenesisButton)index));
            snprintf(value,96,"%s",m->capture==index?"PRESS A KEY...":SDL_GetScancodeName((SDL_Scancode)p->key[index]));
        } else snprintf(label,96,"%s",index==8?"RESTORE KEYBOARD DEFAULTS":"BACK");
    } else if(m->page==PAGE_PAD) {
        if(index==0) {
            snprintf(label,96,"CONTROLLER INPUT");snprintf(value,96,"%s",p->device&INPUT_DEV_GAMEPAD?"ENABLED":"DISABLED");
        } else if(index==1) {
            snprintf(label,96,"STICK DEAD ZONE");snprintf(value,96,"%d%%",p->deadzone_pct);
        } else if(index<10) {
            snprintf(label,96,"%s",input_button_name((GenesisButton)(index-2)));
            if(m->capture==index-2)snprintf(value,96,"PRESS BUTTON/AXIS...");
            else pad_label(&p->pad[index-2],value,96);
        } else snprintf(label,96,"%s",index==10?"RESTORE CONTROLLER DEFAULTS":"BACK");
    } else if(m->page==PAGE_RESET)snprintf(label,96,"%s",index?"RESET GAME":"KEEP PLAYING");
    else if(m->page==PAGE_REPLACE)snprintf(label,96,"%s",index?"REPLACE SAVED STATE":"CANCEL");
    else snprintf(label,96,"BACK");
}
static void draw(Menu *m,SDL_Texture *texture) {
    for(int y=0;y<H;++y) {
        unsigned blue=25+(unsigned)y*12/H;
        for(int x=0;x<W;++x)s_pixels[y*W+x]=0xFF080F00u|blue;
    }
    fill(0,0,W,6,0xFFCBA85Bu);fill(48,113,864,1,0xFF5A4D32u);
    text(64,37,"WOTES RECOMP",0xFFE7C577u,3);
    text(66,76,"WARRIORS OF THE ETERNAL SUN",0xFFB6C6D3u,1);
    text(704,44,"MENU PREVIEW",0xFFB6C6D3u,1);
    text(704,64,"CHILI PATCH V2.0",0xFF8A9DADu,1);
    if(m->page==PAGE_HOME) {
        text(112,145,m->playing?"GAME PAUSED":"WELCOME BACK, ADVENTURER",0xFFCDDBE7u,2);
    } else {
        const char *heading=m->page==PAGE_SETTINGS?"SETTINGS":m->page==PAGE_KEYS?"KEYBOARD CONTROLS":
            m->page==PAGE_PAD?"CONTROLLER CONTROLS":m->page==PAGE_HELP?"CONTROLS GUIDE":
            m->page==PAGE_SAVE?"SAVE STATE":m->page==PAGE_LOAD?"LOAD STATE":
            m->page==PAGE_REPLACE?"REPLACE SAVED STATE?":"RESET THE GAME?";
        text(64,103,heading,0xFFE7C577u,1);
    }
    if(m->page==PAGE_HELP) {
        text(80,147,"ARROWS / D-PAD: NAVIGATE MENUS",0xFFD8E3EBu,2);
        text(80,182,"ENTER / SOUTH BUTTON: SELECT",0xFFD8E3EBu,2);
        text(80,217,"ESC / EAST BUTTON: BACK",0xFFD8E3EBu,2);
        text(80,252,"ESC / BACK-VIEW: PAUSE OR RESUME",0xFFD8E3EBu,2);
        text(80,303,"F5: QUICK SAVE    F6: QUICK LOAD",0xFFB5C4D2u,2);
        text(80,338,"PAUSE MENU: FOUR MANUAL STATE SLOTS",0xFFB5C4D2u,2);
        text(80,373,"F11 / ALT+ENTER: FULLSCREEN",0xFFB5C4D2u,2);
        text(80,408,"HOLD TAB: TURBO (SILENT)",0xFFB5C4D2u,2);
        text(80,453,"GAME BUTTONS ARE EDITABLE IN SETTINGS.",0xFF8FA5B7u,1);
        text(80,470,"QUICK SAVE USES ITS OWN SLOT, APART FROM STATES 1-4.",0xFF8FA5B7u,1);
        fill(64,488,832,38,0xFF253E52u);text(80,499,"BACK",0xFFFFDC8Au,2);
    } else {
        if(m->page==PAGE_RESET) {
            text(112,182,"RETURN TO THE GAME'S BEGINNING?",0xFFD8E3EBu,2);
            text(112,228,"UNSAVED PLAY WILL BE LOST.",0xFFDBBC7Cu,2);
            text(112,264,"YOUR SAVED FILES WILL BE KEPT.",0xFF9FB3C3u,1);
        }
        if(m->page==PAGE_REPLACE) {
            char label[80];
            if(m->save_slot==WOTES_QUICK_SLOT)snprintf(label,sizeof label,"REPLACE THE QUICK SAVE?");
            else snprintf(label,sizeof label,"REPLACE STATE %d?",m->save_slot);
            text(112,182,label,0xFFD8E3EBu,2);
            text(112,228,"THE PREVIOUS STATE WILL BE REPLACED.",0xFFDBBC7Cu,2);
            text(112,264,"IN-GAME SAVES ARE SEPARATE.",0xFF9FB3C3u,1);
        }
        for(int i=0;i<count(m);++i) {
            int y=row_y(m)+i*row_h(m),h=row_h(m)-5;
            fill(64,y,832,h,i==m->selected?0xFF253E52u:0xFF111F2Cu);
            if(i==m->selected)fill(64,y,4,h,0xFFE7C577u);
            char label[96],value[96];row_label(m,i,label,value);
            int text_y=y+(h-16)/2;
            text(86,text_y,label,i==m->selected?0xFFFFDC8Au:0xFFD5E0E8u,2);
            if(*value)text(548,text_y,value,m->capture>=0&&i==m->selected?0xFF74D8B7u:0xFF9EBACC,2);
            if(m->page==PAGE_SETTINGS && i==3) {
                fill(690,y+h/2-2,182,5,0xFF3D5060u);
                fill(690,y+h/2-2,182*g_app_config.volume/100,5,s_muted?0xFF7C8791u:0xFF74D8B7u);
            }
        }
    }
    const char *tip="UP/DOWN: MOVE   LEFT/RIGHT: CHANGE   ENTER: SELECT   ESC: BACK";
    if(m->page==PAGE_SETTINGS && m->selected==1)tip="INTRO CHANGES APPLY WHEN YOU START OR RESET THE GAME.";
    if(m->page==PAGE_KEYS)tip="SELECT AN ACTION, THEN PRESS A KEY. DUPLICATES SWAP.";
    if(m->page==PAGE_PAD)tip="SOUTH: SELECT   EAST: BACK   BACK/VIEW: MENU";
    if(m->page==PAGE_SAVE)tip="FOUR MANUAL SLOTS + A SEPARATE QUICK SAVE. SAVING RESUMES PLAY.";
    if(m->page==PAGE_LOAD)tip="SELECT A SAVED STATE TO RESUME. IN-GAME SAVES STAY SEPARATE.";
    if(m->capture>=0)tip="PRESS YOUR NEW INPUT. ESC OR BACK/VIEW CANCELS.";
    text(64,538,tip,0xFF8FA5B7u,1);
    if(s_notice_until && (Sint32)(s_notice_until-SDL_GetTicks())>0)text(64,558,s_notice,0xFF74D8B7u,1);
    char device[112];const char *name=gamepad_player_name(0);
    snprintf(device,sizeof device,"CONTROLLER: %.70s",name?name:"NOT CONNECTED - PLUG IN AT ANY TIME");
    text(64,579,device,0xFF8FA5B7u,1);
    SDL_UpdateTexture(texture,NULL,s_pixels,W*4);
    SDL_SetRenderDrawColor(m->renderer,8,15,25,255);SDL_RenderClear(m->renderer);
    SDL_RenderCopy(m->renderer,texture,NULL,NULL);SDL_RenderPresent(m->renderer);
}
static void finish(Menu *m,int result) { m->closing=1;m->result=result; }
static void change(Menu *m,int direction) {
    if(m->page==PAGE_SETTINGS) {
        switch(m->selected) {
            case 0:g_app_config.custom_widescreen=!g_app_config.custom_widescreen;g_app_config.custom_aspect=1;break;
            case 1:s_intro=!s_intro;break;
            case 2: {
                int on=!g_app_config.fullscreen;
                if(SDL_SetWindowFullscreen(m->window,on?SDL_WINDOW_FULLSCREEN_DESKTOP:0)!=0) {
                    notice("COULD NOT CHANGE FULLSCREEN MODE");return;
                }
                g_app_config.fullscreen=on;break;
            }
            case 3:g_app_config.volume=clamp(g_app_config.volume+direction*5,0,100);break;
            case 4:s_muted=!s_muted;break;
            default:return;
        }
        wotes_menu_apply();save_settings();
    } else if(m->page==PAGE_PAD && m->selected<2) {
        if(m->selected==0)g_input_map.p[0].device^=INPUT_DEV_GAMEPAD;
        else g_input_map.p[0].deadzone_pct=clamp(g_input_map.p[0].deadzone_pct+direction*5,5,75);
        save_settings();
    }
}
static void activate(Menu *m) {
    if(m->page==PAGE_HOME) {
        if(m->selected==0)finish(m,WOTES_MENU_RESUME);
        else if(m->playing && m->selected==1)page(m,PAGE_SAVE,0);
        else if(m->selected==(m->playing?2:1))page(m,PAGE_LOAD,0);
        else if(m->playing && m->selected==3)page(m,PAGE_RESET,0);
        else if(m->selected==count(m)-1)finish(m,WOTES_MENU_EXIT);
        else page(m,PAGE_SETTINGS,0);
    } else if(m->page==PAGE_SAVE) {
        if(m->selected==5){page(m,PAGE_HOME,1);return;}
        m->save_slot=m->selected+1;
        if(slot_info(m->save_slot,NULL,0))page(m,PAGE_REPLACE,0);
        else finish(m,WOTES_MENU_SAVE_BASE+m->save_slot);
    } else if(m->page==PAGE_LOAD) {
        if(m->selected==5){page(m,PAGE_HOME,m->playing?2:1);return;}
        if(!slot_info(m->selected+1,NULL,0)){notice("THAT SLOT IS EMPTY");return;}
        if(wotes_runner_load_slot(m->selected+1)) {
            trace("STATE_LOADED",m->selected+1);finish(m,WOTES_MENU_RESUME);
        } else notice("COULD NOT LOAD STATE - DAMAGED OR INCOMPATIBLE");
    } else if(m->page==PAGE_REPLACE) {
        if(m->selected)finish(m,WOTES_MENU_SAVE_BASE+m->save_slot);
        else page(m,PAGE_SAVE,m->save_slot-1);
    } else if(m->page==PAGE_SETTINGS) {
        if(m->selected<5)change(m,1);
        else if(m->selected==5)page(m,PAGE_KEYS,0);
        else if(m->selected==6)page(m,PAGE_PAD,0);
        else if(m->selected==7)page(m,PAGE_HELP,0);
        else page(m,PAGE_HOME,m->playing?4:2);
    } else if(m->page==PAGE_KEYS) {
        if(m->selected<8){m->capture=m->selected;notice("ESC, F-KEYS AND TAB ARE RESERVED FOR SHORTCUTS");}
        else if(m->selected==8) {
            const int keys[]={SDL_SCANCODE_UP,SDL_SCANCODE_DOWN,SDL_SCANCODE_LEFT,SDL_SCANCODE_RIGHT,
                SDL_SCANCODE_Z,SDL_SCANCODE_X,SDL_SCANCODE_C,SDL_SCANCODE_RETURN};
            for(int i=0;i<8;++i)g_input_map.p[0].key[i]=keys[i];save_settings();
        } else page(m,PAGE_SETTINGS,5);
    } else if(m->page==PAGE_PAD) {
        if(m->selected<2)change(m,1);
        else if(m->selected<10) {
            if(!gamepad_player_connected(0))notice("CONNECT A CONTROLLER BEFORE REMAPPING");
            else m->capture=m->selected-2;
        } else if(m->selected==10) {
            const int buttons[]={SDL_CONTROLLER_BUTTON_DPAD_UP,SDL_CONTROLLER_BUTTON_DPAD_DOWN,
                SDL_CONTROLLER_BUTTON_DPAD_LEFT,SDL_CONTROLLER_BUTTON_DPAD_RIGHT,SDL_CONTROLLER_BUTTON_B,
                SDL_CONTROLLER_BUTTON_A,SDL_CONTROLLER_BUTTON_X,SDL_CONTROLLER_BUTTON_START};
            for(int i=0;i<8;++i){GamepadBind bind={GP_BIND_BUTTON,buttons[i],0};g_input_map.p[0].pad[i]=bind;}
            g_input_map.p[0].device|=INPUT_DEV_GAMEPAD;g_input_map.p[0].deadzone_pct=25;save_settings();
        } else page(m,PAGE_SETTINGS,6);
    } else if(m->page==PAGE_RESET) {
        if(m->selected)finish(m,WOTES_MENU_RESET);else page(m,PAGE_HOME,0);
    } else page(m,PAGE_SETTINGS,7);
}
static void back(Menu *m,int pause_key) {
    if(m->capture>=0){m->capture=-1;notice("BINDING CANCELLED");return;}
    if(m->page==PAGE_HOME) {
        if(m->playing)finish(m,WOTES_MENU_RESUME);
        return;
    }
    if(pause_key && m->playing){finish(m,WOTES_MENU_RESUME);return;}
    if(m->page==PAGE_SETTINGS)page(m,PAGE_HOME,m->playing?4:2);
    else if(m->page==PAGE_SAVE)page(m,PAGE_HOME,1);
    else if(m->page==PAGE_LOAD)page(m,PAGE_HOME,m->playing?2:1);
    else if(m->page==PAGE_REPLACE)page(m,PAGE_SAVE,m->save_slot-1);
    else if(m->page==PAGE_KEYS)page(m,PAGE_SETTINGS,5);
    else if(m->page==PAGE_PAD)page(m,PAGE_SETTINGS,6);
    else if(m->page==PAGE_HELP)page(m,PAGE_SETTINGS,7);
    else page(m,PAGE_HOME,0);
}
static int reserved(SDL_Scancode sc) {
    return sc==SDL_SCANCODE_ESCAPE || sc==SDL_SCANCODE_TAB || (sc>=SDL_SCANCODE_F1 && sc<=SDL_SCANCODE_F12) ||
        sc==SDL_SCANCODE_LGUI || sc==SDL_SCANCODE_RGUI || sc==SDL_SCANCODE_LALT || sc==SDL_SCANCODE_RALT ||
        sc==SDL_SCANCODE_LCTRL || sc==SDL_SCANCODE_RCTRL || sc==SDL_SCANCODE_UNKNOWN;
}
static int equal_bind(GamepadBind a,GamepadBind b) { return a.kind==b.kind&&a.code==b.code&&a.axis_dir==b.axis_dir; }
static void capture_key(Menu *m,SDL_Scancode key) {
    if(key==SDL_SCANCODE_ESCAPE){back(m,0);return;}
    if(reserved(key)){notice("THAT KEY IS RESERVED - CHOOSE ANOTHER");return;}
    PlayerInput *p=&g_input_map.p[0];int previous=p->key[m->capture];
    for(int i=0;i<8;++i)if(i!=m->capture && p->key[i]==key)p->key[i]=previous;
    p->key[m->capture]=key;m->capture=-1;save_settings();
}
static void capture_pad(Menu *m,GamepadBind bind) {
    if(bind.kind==GP_BIND_BUTTON && (bind.code==SDL_CONTROLLER_BUTTON_BACK || bind.code==SDL_CONTROLLER_BUTTON_GUIDE)) {
        back(m,0);return;
    }
    PlayerInput *p=&g_input_map.p[0];GamepadBind previous=p->pad[m->capture];
    for(int i=0;i<8;++i)if(i!=m->capture && equal_bind(p->pad[i],bind))p->pad[i]=previous;
    p->pad[m->capture]=bind;m->capture=-1;save_settings();
}
static void command(Menu *m,int cmd) {
    if(cmd==0)m->selected=(m->selected+count(m)-1)%count(m);
    if(cmd==1)m->selected=(m->selected+1)%count(m);
    if(cmd==2)change(m,-1);
    if(cmd==3)change(m,1);
    if(cmd==4)activate(m);
    if(cmd==5)back(m,0);
}
static int all_released(void) {
    const Uint8 *keys=SDL_GetKeyboardState(NULL);
    for(int i=0;i<SDL_NUM_SCANCODES;++i)if(keys[i])return 0;
    SDL_GameController *pad=SDL_GameControllerFromInstanceID(gamepad_player_instance(0));
    if(pad) {
        for(int i=0;i<SDL_CONTROLLER_BUTTON_MAX;++i)if(SDL_GameControllerGetButton(pad,(SDL_GameControllerButton)i))return 0;
        for(int i=0;i<4;++i)if(abs(SDL_GameControllerGetAxis(pad,(SDL_GameControllerAxis)i))>16000)return 0;
    }
    return 1;
}
int wotes_menu_run(SDL_Renderer *renderer,SDL_Window *window,int playing) {
    Menu m={0};m.renderer=renderer;m.window=window;m.playing=playing;m.capture=-1;
    int old_w,old_h;SDL_RenderGetLogicalSize(renderer,&old_w,&old_h);
    SDL_bool old_integer=SDL_RenderGetIntegerScale(renderer);
    SDL_RenderSetIntegerScale(renderer,SDL_FALSE);SDL_RenderSetLogicalSize(renderer,W,H);
    SDL_Texture *texture=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,W,H);
    if(!texture){SDL_RenderSetLogicalSize(renderer,old_w,old_h);return WOTES_MENU_EXIT;}
    SDL_SetTextureScaleMode(texture,SDL_ScaleModeNearest);
    int was_blocked=input_gameplay_blocked();input_set_gameplay_blocked(1);
    audio_set_playback_enabled(0);audio_discard_playback();wotes_menu_apply();
    trace("OPEN",playing);s_notice_until=0;
    int done=0;
    while(!done) {
        wotes_menu_test_tick(1);
        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            gamepad_handle_event(&e);
            if(e.type==SDL_QUIT){m.result=WOTES_MENU_EXIT;done=1;break;}
            if(e.type==SDL_RENDER_DEVICE_RESET||e.type==SDL_RENDER_TARGETS_RESET) {
                m.renderer_reset=1;
                SDL_DestroyTexture(texture);
                texture=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,W,H);
                if(!texture){m.result=WOTES_MENU_EXIT;done=1;break;}
            }
            if(m.closing)continue;
            if(e.type==SDL_KEYDOWN && !e.key.repeat) {
                SDL_Scancode sc=e.key.keysym.scancode;
                if(m.capture>=0) {
                    if(m.page==PAGE_KEYS)capture_key(&m,sc);
                    else if(sc==SDL_SCANCODE_ESCAPE)back(&m,0);
                    continue;
                }
                if(sc==SDL_SCANCODE_F5) {
                    if(m.playing)finish(&m,WOTES_MENU_SAVE_BASE+WOTES_QUICK_SLOT);
                    else notice("START OR LOAD A GAME BEFORE QUICK SAVING");
                    continue;
                }
                if(sc==SDL_SCANCODE_F6) {
                    if(!slot_info(WOTES_QUICK_SLOT,NULL,0))notice("NO QUICK SAVE YET - USE F5 WHILE PLAYING");
                    else if(wotes_runner_load_slot(WOTES_QUICK_SLOT)) {
                        trace("STATE_LOADED",WOTES_QUICK_SLOT);finish(&m,WOTES_MENU_RESUME);
                    } else notice("COULD NOT LOAD QUICK SAVE - DAMAGED OR INCOMPATIBLE");
                    continue;
                }
                if(sc==SDL_SCANCODE_ESCAPE){back(&m,0);continue;}
                if(sc==SDL_SCANCODE_F11 || (sc==SDL_SCANCODE_RETURN && e.key.keysym.mod&KMOD_ALT)) {
                    int selected=m.selected,pg=m.page;m.page=PAGE_SETTINGS;m.selected=2;change(&m,1);m.page=pg;m.selected=selected;continue;
                }
                if(sc==SDL_SCANCODE_UP)command(&m,0);
                if(sc==SDL_SCANCODE_DOWN)command(&m,1);
                if(sc==SDL_SCANCODE_LEFT)command(&m,2);
                if(sc==SDL_SCANCODE_RIGHT)command(&m,3);
                if(sc==SDL_SCANCODE_RETURN||sc==SDL_SCANCODE_SPACE)command(&m,4);
                if(sc==SDL_SCANCODE_BACKSPACE)command(&m,5);
            }
            if(e.type==SDL_CONTROLLERDEVICEREMOVED && m.capture>=0 && m.page==PAGE_PAD) {
                m.capture=-1;notice("CONTROLLER DISCONNECTED - PLUG IN TO CONTINUE");
            }
            if(e.type==SDL_CONTROLLERBUTTONDOWN && e.cbutton.which==gamepad_player_instance(0)) {
                int b=e.cbutton.button;
                if(m.capture>=0) {
                    if(m.page==PAGE_PAD){GamepadBind bind={GP_BIND_BUTTON,b,0};capture_pad(&m,bind);}
                    else if(b==SDL_CONTROLLER_BUTTON_BACK)back(&m,0);
                    continue;
                }
                if(b==SDL_CONTROLLER_BUTTON_BACK){back(&m,1);continue;}
                if(b==SDL_CONTROLLER_BUTTON_DPAD_UP)command(&m,0);
                if(b==SDL_CONTROLLER_BUTTON_DPAD_DOWN)command(&m,1);
                if(b==SDL_CONTROLLER_BUTTON_DPAD_LEFT)command(&m,2);
                if(b==SDL_CONTROLLER_BUTTON_DPAD_RIGHT)command(&m,3);
                if(b==SDL_CONTROLLER_BUTTON_A)command(&m,4);
                if(b==SDL_CONTROLLER_BUTTON_B)command(&m,5);
            }
            if(e.type==SDL_CONTROLLERAXISMOTION && e.caxis.which==gamepad_player_instance(0)) {
                if(m.capture>=0) {
                    if(m.page==PAGE_PAD && abs(e.caxis.value)>22000) {
                        GamepadBind bind={GP_BIND_AXIS,e.caxis.axis,e.caxis.value<0?-1:1};capture_pad(&m,bind);
                    }
                    continue;
                }
                int d=e.caxis.value>18000?1:e.caxis.value< -18000?-1:0;
                if(e.caxis.axis==SDL_CONTROLLER_AXIS_LEFTY) {
                    if(d && d!=m.axis_y)command(&m,d>0?1:0);m.axis_y=d;
                }
                if(e.caxis.axis==SDL_CONTROLLER_AXIS_LEFTX) {
                    if(d && d!=m.axis_x)command(&m,d>0?3:2);m.axis_x=d;
                }
            }
            if((e.type==SDL_MOUSEMOTION || e.type==SDL_MOUSEBUTTONDOWN) && m.capture<0) {
                int x=e.type==SDL_MOUSEMOTION?e.motion.x:e.button.x;
                int y=e.type==SDL_MOUSEMOTION?e.motion.y:e.button.y;
                int top=m.page==PAGE_HELP?488:row_y(&m),height=m.page==PAGE_HELP?38:row_h(&m);
                int index=(y-top)/height;
                if(x>=64 && x<896 && y>=top && index>=0 && index<count(&m)) {
                    m.selected=index;
                    if(e.type==SDL_MOUSEBUTTONDOWN && e.button.button==SDL_BUTTON_LEFT) {
                        if(m.page==PAGE_SETTINGS && index==3 && x>=690) {
                            int pct=clamp((x-690)*100/182,0,100);
                            g_app_config.volume=clamp(((pct+2)/5)*5,0,100);wotes_menu_apply();save_settings();
                        } else activate(&m);
                    }
                }
            }
        }
        if(m.closing && all_released())done=1;
        draw(&m,texture);SDL_Delay(10);
    }
    (void)gamepad_consume_quicksave();(void)gamepad_consume_quickload();
    audio_discard_playback();SDL_DestroyTexture(texture);
    SDL_RenderSetIntegerScale(renderer,old_integer);SDL_RenderSetLogicalSize(renderer,old_w,old_h);
    input_set_gameplay_blocked(was_blocked);trace("CLOSE",m.result);
    if(m.renderer_reset){SDL_Event e;memset(&e,0,sizeof e);e.type=SDL_RENDER_DEVICE_RESET;SDL_PushEvent(&e);}
    return m.result;
}

/* Local integration-test driver. Events exercise the same UI handlers as real
 * keys and a virtual SDL controller, without modifying gameplay internals. */
static FILE *s_test_script;
static int s_test_initialized,s_test_wait,s_test_context;
static SDL_Joystick *s_test_pad;
static int s_test_device_index=-1;
void wotes_menu_test_device(void) {
    if(!getenv("WOTES_MENU_TEST_SCRIPT") || !getenv("WOTES_MENU_VIRTUAL_PAD") || s_test_pad)return;
    SDL_VirtualJoystickDesc desc;memset(&desc,0,sizeof desc);
    desc.version=SDL_VIRTUAL_JOYSTICK_DESC_VERSION;desc.type=SDL_JOYSTICK_TYPE_GAMECONTROLLER;
    desc.naxes=SDL_CONTROLLER_AXIS_MAX;desc.nbuttons=SDL_CONTROLLER_BUTTON_MAX;
    desc.button_mask=(1u<<SDL_CONTROLLER_BUTTON_MAX)-1;desc.axis_mask=(1u<<SDL_CONTROLLER_AXIS_MAX)-1;
    desc.name="WotES virtual test controller";
    s_test_device_index=SDL_JoystickAttachVirtualEx(&desc);
    if(s_test_device_index>=0)s_test_pad=SDL_JoystickOpen(s_test_device_index);
}
static void test_fail(const char *line) { fprintf(stderr,"[MENU-TEST] FAILED: %s\n",line);exit(4); }
void wotes_menu_test_tick(int in_menu) {
    if(!s_test_initialized) {
        const char *path=getenv("WOTES_MENU_TEST_SCRIPT");s_test_initialized=1;
        if(path && *path){s_test_script=fopen(path,"r");if(!s_test_script)test_fail("cannot open script");}
    }
    if(!s_test_script)return;
    if(s_test_wait>0){if(in_menu==s_test_context)--s_test_wait;return;}
    char line[2300],cmd[32],arg[2200];
    while(fgets(line,sizeof line,s_test_script)) {
        cmd[0]=arg[0]=0;
        int parts=sscanf(line,"%31s %2199[^\r\n]",cmd,arg);
        if(parts<1 || cmd[0]=='#')continue;
        if(!strcmp(cmd,"WAIT_MENU") || !strcmp(cmd,"WAIT_GAME")) {
            s_test_wait=atoi(arg);s_test_context=!strcmp(cmd,"WAIT_MENU");return;
        }
        if(!strcmp(cmd,"KEY")) {
            SDL_Scancode sc=SDL_GetScancodeFromName(arg);if(!sc)test_fail(line);
            SDL_Event e;memset(&e,0,sizeof e);e.type=SDL_KEYDOWN;e.key.keysym.scancode=sc;e.key.keysym.sym=SDL_GetKeyFromScancode(sc);
            SDL_PushEvent(&e);e.type=SDL_KEYUP;SDL_PushEvent(&e);return;
        }
        if(!strcmp(cmd,"CLICK")) {
            SDL_Event e;memset(&e,0,sizeof e);e.type=SDL_MOUSEBUTTONDOWN;e.button.button=SDL_BUTTON_LEFT;
            if(sscanf(arg,"%d %d",&e.button.x,&e.button.y)!=2)test_fail(line);
            SDL_PushEvent(&e);return;
        }
        if(!strcmp(cmd,"PAD_DOWN") || !strcmp(cmd,"PAD_UP")) {
            SDL_GameControllerButton b=SDL_GameControllerGetButtonFromString(arg);
            if(!s_test_pad || b==SDL_CONTROLLER_BUTTON_INVALID)test_fail(line);
            SDL_JoystickSetVirtualButton(s_test_pad,b,!strcmp(cmd,"PAD_DOWN"));SDL_JoystickUpdate();return;
        }
        if(!strcmp(cmd,"AXIS")) {
            char name[48];int value;
            if(!s_test_pad || sscanf(arg,"%47s %d",name,&value)!=2)test_fail(line);
            SDL_GameControllerAxis axis=SDL_GameControllerGetAxisFromString(name);
            if(axis==SDL_CONTROLLER_AXIS_INVALID)test_fail(line);
            SDL_JoystickSetVirtualAxis(s_test_pad,axis,(Sint16)value);SDL_JoystickUpdate();return;
        }
        if(!strcmp(cmd,"UNPLUG")) {
            if(s_test_pad){SDL_JoystickClose(s_test_pad);s_test_pad=NULL;SDL_JoystickDetachVirtual(s_test_device_index);}return;
        }
        if(!strcmp(cmd,"PLUG")){wotes_menu_test_device();return;}
        if(!strcmp(cmd,"SNAPSHOT")) {
            if(!in_menu || png_write_argb(arg,s_pixels,W,H,W)!=0)test_fail(line);
        } else if(!strcmp(cmd,"DUMP_RAM")) {
            FILE *f=fopen(arg,"wb");if(!f)test_fail(line);fwrite(g_ram,1,sizeof g_ram,f);fclose(f);
        } else if(!strcmp(cmd,"INPUT")) {
            fprintf(stderr,"[MENU-TEST] input=%04x expected=%s\n",input_current_mask(0),arg);
            if(input_current_mask(0)!=(uint16_t)strtoul(arg,NULL,16))test_fail(line);
        } else if(!strcmp(cmd,"QUIT")) {
            SDL_Event e;memset(&e,0,sizeof e);e.type=SDL_QUIT;SDL_PushEvent(&e);return;
        } else test_fail(line);
    }
    fclose(s_test_script);s_test_script=NULL;
}
