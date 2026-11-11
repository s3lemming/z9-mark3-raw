#include "hsfc_geometry_mode.h"
uint32_t m3_hsfc_geometry_mode(uint32_t mode,uint32_t area){
 if(mode==3u&&area==1u)return 5u;
 if(mode!=6u)return mode;
 return area==0u?3u:area==1u?5u:6u;
}
