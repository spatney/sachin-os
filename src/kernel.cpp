#include <common/types.h>
#include <common/console.h>

#include <hardware/video.h>
#include <hardware/pci.h>
#include <hardware/interrupts.h>

#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>

#include <gui/graphics.h>
#include <gui/desktop.h>
#include <gui/window.h>

#include <gdt.h>

using namespace moos;
using namespace moos::hardware;
using namespace moos::drivers;
using namespace moos::common;
using namespace moos::gui;

class PrintFKeyboardEventHandler : public KeyboardEventHandler
{
public:
    void OnKeyDown(int8_t c)
    {
        if (c == 0X0E)
        {
            Console::Backspace();
        }
        else
        {
            Console::Write("%c", c);
        }
    }
};

class MouseToCosole : public MouseEventHandler
{
    int8_t x, y;
    Desktop *desktop;
    GraphicsContext *gc;

public:
    MouseToCosole(Desktop *desktop, GraphicsContext *gc)
    {
        /*x = 40;
        y = 12;
        invertVideoMemoryAt(x, y);*/

        this->desktop = desktop;
        this->gc = gc;
    }
    void OnMouseDown(uint8_t button)
    {
        //invertVideoMemoryAt(x, y);

        desktop->OnMouseDown(button);
    }
    void OnMouseUp(uint8_t button)
    {
        //invertVideoMemoryAt(x, y);
        desktop->OnMouseUp(button);
    }
    void OnMouseMove(int32_t xOffset, int32_t yOffSet)
    {
        /*invertVideoMemoryAt(x, y);

        x += xOffset;
        if (x >= 80)
        {
            x = 79;
        }
        if (x < 0)
        {
            x = 0;
        }
        y += yOffSet;
        if (y >= 25)
        {
            y = 24;
        }
        if (y < 0)
        {
            y = 0;
        }

        invertVideoMemoryAt(x, y);*/

        desktop->OnMouseMove(xOffset, yOffSet);
        //desktop->Draw(gc);
    }
    void invertVideoMemoryAt(int8_t x, int8_t y)
    {
        VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0x0F00) << 4 | (VideoMemory[80 * y + x] & 0xF000) >> 4 | (VideoMemory[80 * y + x] & 0x00FF);
    }
};

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for (constructor *i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

void DrawDummyDesktop(GraphicsContext *gc)
{
    uint32_t w = 320;
    uint32_t h = 200;

    gc->FillRectangle(0, 0, w, h, 0x00, 0x00, 0x00);
    gc->FillRectangle(w / 4, h / 4, 100, 100, 0x00, 0x00, 0xA8);
    gc->FillRectangle(w / 6, h / 6, 50, 70, 0x00, 0xA8, 0x00);
    gc->FillRectangle(w * 2 / 3, h * 1 / 2 - 20, 100, 50, 0xA8, 0x00, 0x00);
    gc->FillRectangle(0, h - 20, w, 20, 0x00, 0x00, 0xA8);

    gc->DrawLine(10, 10, 100, 10, 0xA8, 0x00, 0x00);
    gc->DrawLine(100, 10, 120, 100, 0xA8, 0x00, 0x00);
    gc->DrawLine(120, 100, 30, 100, 0xA8, 0x00, 0x00);
    gc->DrawLine(30, 100, 10, 10, 0xA8, 0x00, 0x00);
}

extern "C" void kernel_main(void *multiboot, uint32_t magic)
{
    GlobalDescriptorTable gdt;
    InterruptManager interruptManager(&gdt);

    Console::Clear();
    Console::Write("Booting MoOS Kernel ...\n\n");
    Console::Write("Initializing driver manager ...\n");

    DriverManager driverManager;
    PeripheralComponentInterconnectController pciController;
    pciController.SelectDrivers(&driverManager, &interruptManager);

    PrintFKeyboardEventHandler keyboardEventHandler;
    KeyboardDriver keyboard(&interruptManager, &keyboardEventHandler);
    driverManager.AddDriver(&keyboard);

    GraphicsContext gc;
    Desktop desktop(100, 100, 0xFF, 0xFF, 0xFF);

    MouseToCosole mouseToConsole(&desktop, &gc);

    MouseDriver mouse(&interruptManager, &mouseToConsole);
    driverManager.AddDriver(&mouse);

    Console::Write("Activating driver manager ...\n");
    driverManager.ActivateAll();

    Console::Write("Activating interrupt manager ...\n");
    interruptManager.Activate();

    Console::Write("\n\nMoOS\a> ");

    //DrawDummyDesktop(&gc);
    //gc.FillRectangle(0, 0, 320, 200, 0xFF, 0xFF, 0xFF);

    Window win1(&desktop, 10,10, 20, 20, 0xA8,0,0);
    Window win2(&desktop, 40,40, 30, 30, 0, 0xA8,0);

    desktop.AddChildWidget(&win1);
    desktop.AddChildWidget(&win2);

    desktop.Draw(&gc);
    
    while (1)
    {
        desktop.Draw(&gc);
    }
}