#ifndef __MOOS__GUI__Terminal_H
#define __MOOS__GUI__Terminal_H

#include <common/types.h>
#include <drivers/mouse.h>
#include <drivers/keyboard.h>

namespace moos
{
    namespace gui
    {
        class Terminal : public drivers::MouseEventHandler, public drivers::KeyboardEventHandler
        {
        private:
            common::int32_t x;
            common::int32_t y;
            common::uint8_t promptY;
            bool isShiftDown;
            bool isCapsLockOn;

            void drawPrompt();
            common::uint8_t KeyToChar(drivers::Key key);

        public:
            Terminal();
            ~Terminal();

            void Reset();
            void OnKeyDown(drivers::Key key);
            void OnKeyUp(drivers::Key key);
            void OnMouseDown(common::uint8_t button);
            void OnMouseUp(common::uint8_t button);

            void OnMouseMove(
                common::int32_t xOffset,
                common::int32_t yOffset);

            void invertVideoMemoryAt(
                common::int8_t x,
                common::int8_t y);
        };
    }
}

#endif