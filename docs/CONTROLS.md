# DualSense controls and widescreen

| Action / DS button | Default DualSense input |
| --- | --- |
| Move | D-pad / left stick |
| Confirm / A | Circle |
| Back / B | Cross |
| X | Triangle |
| Y | Square |
| L / R | L1 / R1 |
| START / game menu | Options |
| SELECT / settings | Create |
| Fast-forward | R2 |
| Exit | Hold Create for three seconds |

At the title screen press Create for System Settings. In gameplay press Options, then Create. Input mappings can be changed in Controls; runtime prompts follow the active mappings. The defaults preserve the DS A/B arrangement. Runtime menus and the identified in-game button art use DualSense symbols and labels. RC3 adds built-in replacements for 97 shared menu, battle, field, warp and race sprite variants, plus 103 explicit button references in 57 English/French instruction strings. START becomes Options, SELECT becomes Create, and shoulder prompts use L1/R1 with the default bindings. The native Controls page uses action descriptions instead of DS button-letter row names.

In-game artwork and instructions use the saved controller bindings at startup. **Fully close and relaunch CTPS5 after rebinding** to refresh those game graphics. Runtime prompts update immediately. Game input behavior and existing bindings are retained. Some scenes may use additional artwork that has not yet been identified; console confirmation of the replacement coverage is pending.

## Widescreen

RC2 enables 16:9 with room-boundary clipping and plain borders. An RC1 update turns its forced room-edge filling off and changes its matching-panel default to Original once; aspect, minimap and crop/stretch choices are retained. Later user choices are retained. Some maps have enough off-screen content for wider rendering, while menus, dialogue, movies and some gameplay scenes have fixed original layouts.

Choose a fixed-screen presentation in System Settings:

| Mode | Result |
| --- | --- |
| Clean panels | Proportional central image, with solid navy side panels |
| Original (default) | Original proportions with plain borders |
| Crop | Fills the screen by cropping; may hide content |
| Stretch | Fills the screen by changing proportions |

Side panels do not create extra map geometry. Blurred copies of scene edges have been removed. Leave Fill screen sides off for room-boundary clipping; enabling it can expose adjacent or wrapped tiles in small interiors. System Settings stays proportional for readable text. Use Original if you prefer the unmodified aspect ratio. Supported wide gameplay areas can still render wider independently of fixed-screen presentation.
