/**
* Comp 3931 - Project
* Adaptive Noise Canceller (Simplified Adaptive Noise Cancelling) with STFT Post-Filter
*
* Authors:
* - Brady Duval
* - Nick Sutton
* - James Smith
* - PapaGPT (Blessed be thy name)... and his little brother gemini <3
*/
#include <juce_gui_extra/juce_gui_extra.h>
#include "MainComponent.h"

class NCliteApplication : public juce::JUCEApplication {
public:
    const juce::String getApplicationName() override { return "NClite"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }


    void initialise(const juce::String&) override {
        mainWindow.reset(new MainWindow(getApplicationName()));
    }
    void shutdown() override { mainWindow = nullptr; }
    void systemRequestedQuit() override { quit(); }


    class MainWindow : public juce::DocumentWindow {
    public:
        explicit MainWindow(juce::String name)
        : juce::DocumentWindow(name,
        juce::Desktop::getInstance().getDefaultLookAndFeel()
        .findColour(juce::ResizableWindow::backgroundColourId),
        DocumentWindow::allButtons) {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(), true);
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
        }
        void closeButtonPressed() override {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }
    };


private:
    std::unique_ptr<MainWindow> mainWindow;
};


START_JUCE_APPLICATION(NCliteApplication)

