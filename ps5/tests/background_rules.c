/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "runtime/background_rules.h"
#include <assert.h>
int main(void){
 BgRules r={0};const char*good="{\"schema\":1,\"transparentTiles\":\"original\",\"includeCharacterSheets\":true,\"spriteBlendWeight\":36,\"locations\":[{\"id\":2},{\"id\":1}]}";
 assert(bg_rules_parse(good,strlen(good),&r)&&r.count==2&&r.characters&&r.sprite_blend==36);assert(bg_rules_index(&r,2)==0&&bg_rules_index(&r,1)==1&&bg_rules_index(&r,3)<0);
 const char*bad[]={"{}","{\"schema\":2}","{\"schema\":1,\"transparentTiles\":\"original\",\"locations\":[]}","{\"schema\":1,\"transparentTiles\":\"original\",\"locations\":[{\"id\":2},{\"id\":2}]}","{\"schema\":1,\"transparentTiles\":\"original\",\"locations\":[{\"id\":2}],\"includeCharacterSheets\":\"true\"}","{\"schema\":1,\"schema\":1,\"transparentTiles\":\"original\",\"locations\":[{\"id\":2}]}","{\"schema\":1,\"transparentTiles\":\"original\",\"locations\":[{\"id\":2,\"paletteLight\":[1,2]}]}"};
 BgRules before=r;for(unsigned i=0;i<sizeof bad/sizeof bad[0];i++){assert(!bg_rules_parse(bad[i],strlen(bad[i]),&r));assert(!memcmp(&before,&r,sizeof r));}assert(!bg_rules_parse(good,BG_RULE_BYTES+1,&r));
 puts("PASS: room-rule parsing, actor opt-in, blend bounds, duplicate/invalid/empty rejection and transactional retention");return 0;
}
