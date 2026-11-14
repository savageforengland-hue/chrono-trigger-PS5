/* Controller-only native profile. Avoid hard-loading optional keyboard/IME
 * modules before the native entry point. DualSense input uses the pad backend.
 * These are the private callback signatures used by the pinned SDL PS5 backend;
 * the optional native-keyboard object is not extracted from its static archive.
 */
#include <SDL3/SDL.h>
typedef struct SDL_VideoDevice SDL_VideoDevice;
bool PS5_Keyboard_Init(void) { return true; }
bool PS5_Keyboard_Open(void) { return true; }
void PS5_Keyboard_PumpEvents(void) {}
void PS5_Keyboard_Close(void) {}
bool PS5_HasScreenKeyboardSupport(SDL_VideoDevice *device) { (void)device; return false; }
void PS5_ShowScreenKeyboard(SDL_VideoDevice *device, SDL_Window *window, SDL_PropertiesID props) {
    (void)device;(void)window;(void)props;
    SDL_SetError("This PS5 profile uses controller input; native keyboard/IME is disabled");
}
void PS5_HideScreenKeyboard(SDL_VideoDevice *device, SDL_Window *window) { (void)device;(void)window; }
