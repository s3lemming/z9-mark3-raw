#include "native_menu_callbacks.h"
#include <stddef.h>
_Static_assert(sizeof(struct m3_native_menu_event)==24,"Native event layout");
_Static_assert(offsetof(struct m3_native_menu_event,target)==12,"Native target offset");
static uint32_t r16(const uint8_t*p){return p[0]|(uint32_t)p[1]<<8;}
static uint32_t r32(const uint8_t*p){return r16(p)|r16(p+2)<<16;}
static uint64_t r64(const uint8_t*p){return r32(p)|(uint64_t)r32(p+4)<<32;}
static int identity(const struct m3_native_menu_ui*ui,const void*ctx){
 return ui&&ui->pages&&ui->pages->magic==0x334d5047&&ui->pages->version==1&&ctx==ui->pages->states[1];
}
static int owned(const struct m3_native_menu_ui*ui,const void*ctx){
 if(!identity(ui,ctx))return 0;
 const struct m3_native_pages*p=ui->pages;
 if(r64(p->states[1]+0x50)!=(uintptr_t)p->holders[1]||r64(p->holders[1]+0x10)!=(uintptr_t)p->rows[1]||r16(p->holders[1]+0x20)!=7||!r64(p->states[1]+0x48))return 0;
 /* Original mark helper has an unbounded second UI lookup. Only call it
  * after proving every exact owned ID is present in this5-row page. */
 for(unsigned i=0;i<7;i++)if(r32(p->rows[1][i]+24)!=(i<5?0x7e80+i:0x7e87+i-5))return 0;
 return 1;
}
static int setting_row(const struct m3_hsfc_settings*s){
 if(!s->generation||(s->codec!=M3_HE&&s->codec!=M3_HE_STAR))return -1;
 for(unsigned i=0;i<M3_HSFC_OPTION_COUNT;i++)if(s->option==m3_hsfc_option_ids[i])return(int)i;
 return -1;
}
static int ready(const struct m3_native_menu_ui*ui){return(ui->contracts&M3_REQUIRED_CONTRACTS)==M3_REQUIRED_CONTRACTS;}
enum m3_menu_ui_result m3_native_c15_refresh(const struct m3_native_menu_ui*ui,void*ctx){
 if(!owned(ui,ctx)||!ui->read_committed||!ui->entry_style||!ui->focus||!ui->mark)return M3_UI_INVALID;
 if(!ready(ui))return M3_UI_NOT_READY;
 struct m3_hsfc_settings s={UINT32_MAX,UINT32_MAX,0}; if(!ui->read_committed(ui->user,&s))return M3_UI_NOT_READY;
 int row=setting_row(&s);if(row<0)return M3_UI_NOT_READY;
 if(ui->entry_style()!=1)ui->focus(ctx,0x7e80+(uint32_t)row);
 for(unsigned i=0;i<5;i++)ui->mark(ctx,0x7e80+i,i==(unsigned)row);
 ui->mark(ctx,0x7e87,s.codec==M3_HE_STAR);ui->mark(ctx,0x7e88,s.codec==M3_HE);
 return M3_UI_APPLIED;
}
enum m3_menu_ui_result m3_native_c15_submit(const struct m3_native_menu_ui*ui,struct m3_native_menu_event*event){
 if(!event||!identity(ui,event->context))return M3_UI_INVALID;
 /* The native dispatcher reads this exact target AFTER the action returns.
  * A rejected or failed setting remains on the page rather than popping as
  * if a new format had been saved. Unknown contexts never reach this write. */
 uint32_t old_target=event->target;event->target=0x6b8;
 if(!owned(ui,event->context))return M3_UI_INVALID;
 if(old_target!=0x6b9||(event->event!=0x298&&event->event!=0x29d&&event->event!=0x2c0)||!ui->read_committed||!ui->busy||!ui->commit)return M3_UI_INVALID;
 if(!ready(ui))return M3_UI_NOT_READY;
 if(ui->busy(ui->user))return M3_UI_BUSY;
 const uint8_t*ctx=event->context;uint32_t row=r16(ctx+0x32)+r16(ctx+0x58);
 if(row>=7)return M3_UI_INVALID;
 struct m3_hsfc_settings s={UINT32_MAX,UINT32_MAX,0};if(!ui->read_committed(ui->user,&s)||setting_row(&s)<0)return M3_UI_NOT_READY;
 uint32_t option=row<5?m3_hsfc_option_ids[row]:s.option;
 uint32_t codec=row<5?s.codec:row==5?M3_HE_STAR:M3_HE;
 struct m3_hsfc_settings next={option,codec,s.generation};
 if(option!=s.option||codec!=s.codec){
  if(s.generation==UINT64_MAX)return M3_UI_INVALID;
  next.generation++;
 }
 if(!ui->commit(ui->user,s.generation,&next))return M3_UI_COMMIT_FAILED;
 event->target=0x6b9;return M3_UI_APPLIED;
}
