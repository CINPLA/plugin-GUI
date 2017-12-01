/*
    ------------------------------------------------------------------

    This file is part of the Tracking plugin for the Open Ephys GUI
    Written by:

    Alessio Buccino     alessiob@ifi.uio.no
    Mikkel Lepperod
    Svenn-Arne Dragly

    Center for Integrated Neuroplasticity CINPLA
    Department of Biosciences
    University of Oslo
    Norway

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PULSEPALOUTPUTCANVAS_H
#define PULSEPALOUTPUTCANVAS_H


#include <VisualizerWindowHeaders.h>
#include "PulsePalOutputEditor.h"
#include "PulsePalOutput.h"

class PulsePalOutputCanvas : public Visualizer,
        public Button::Listener,
        public Label::Listener,
        public KeyListener
{
public:
    PulsePalOutputCanvas(PulsePalOutput* PulsePalOutput);
    ~PulsePalOutputCanvas();

    void paint (Graphics&);
    void resized();
    void initButtons();
    void initLabels();

    PulsePalOutput* getProcessor();

    // KeyListener interface
    virtual bool keyPressed(const KeyPress &key, Component *originatingComponent);

    // Listener interface
    virtual void buttonClicked(Button* button);
    virtual void labelTextChanged(Label *label) override;

    // Visualizer interface
    virtual void refreshState();
    virtual void update();
    virtual void refresh();
    virtual void beginAnimation();
    virtual void endAnimation();
    virtual void setParameter(int, float);
    virtual void setParameter(int, int, int, float);

    //    int getSelectedSource() const;


private:
    PulsePalOutput* processor;
    ScopedPointer<Label> pulsePalLabel;

    ScopedPointer<UtilityButton> negFirstButton[PULSEPALCHANNELS];
    ScopedPointer<UtilityButton> posFirstButton[PULSEPALCHANNELS];
    ScopedPointer<UtilityButton> biphasicButton[PULSEPALCHANNELS];
    ScopedPointer<UtilityButton> ttlButton[PULSEPALCHANNELS];

    ScopedPointer<Label> channelLabel[PULSEPALCHANNELS];
    ScopedPointer<Label> phase1Label[PULSEPALCHANNELS];
    ScopedPointer<Label> phase2Label[PULSEPALCHANNELS];
    ScopedPointer<Label> interphaseLabel[PULSEPALCHANNELS];
    ScopedPointer<Label> voltage1Label[PULSEPALCHANNELS];
    ScopedPointer<Label> voltage2Label[PULSEPALCHANNELS];
    ScopedPointer<Label> interpulseLabel[PULSEPALCHANNELS];
    ScopedPointer<Label> repetitionsLabel[PULSEPALCHANNELS];
    ScopedPointer<Label> trainDurationLabel[PULSEPALCHANNELS];

    ScopedPointer<Label> phase1EditLabel[PULSEPALCHANNELS];
    ScopedPointer<Label> phase2EditLabel[PULSEPALCHANNELS];
    ScopedPointer<Label> interphaseEditLabel[PULSEPALCHANNELS];
    ScopedPointer<Label> voltage1EditLabel[PULSEPALCHANNELS];
    ScopedPointer<Label> voltage2EditLabel[PULSEPALCHANNELS];
    ScopedPointer<Label> interpulseEditLabel[PULSEPALCHANNELS];
    ScopedPointer<Label> repetitionsEditLabel[PULSEPALCHANNELS];
    ScopedPointer<Label> trainDurationEditLabel[PULSEPALCHANNELS];

    Colour labelColour;
    Colour labelTextColour;
    Colour labelBackgroundColour;

    float my_round(float x);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PulsePalOutputCanvas);
};


#endif // PULSEPALOUTPUTCANVAS_H
