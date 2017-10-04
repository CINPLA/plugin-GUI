/*
  ==============================================================================

    TrackingVisualizer.h
    Created: 5 Oct 2015 11:34:58am
    Author:  mikkel

  ==============================================================================
*/

#ifndef TRACKINGVISUALIZER_H_INCLUDED
#define TRACKINGVISUALIZER_H_INCLUDED

#include <ProcessorHeaders.h>
#include "TrackingVisualizerEditor.h"

#include <vector>

#define MAX_SOURCES 4

//==============================================================================
/*
*/
class TrackingVisualizer : public GenericProcessor
{
public:
    TrackingVisualizer();
    ~TrackingVisualizer();

    AudioProcessorEditor* createEditor();

    void process(AudioSampleBuffer& buffer) override;
    void handleEvent (const EventChannel* eventInfo, const MidiMessage& event, int) override;

    float getX(int s) const;
    float getY(int s) const;
    float getWidth(int s) const;
    float getHeight(int s) const;
    bool getIsRecording() const;
    bool getClearTracking() const;

    int getNSources() const;

    void setClearTracking(bool clear);

    void clearPositionUpdated();
    bool positionIsUpdated() const;

//    bool isSink(); //get the color correct

private:
    std::vector<float> m_x;
    std::vector<float> m_y;
    std::vector<float> m_width;
    std::vector<float> m_height;
    bool m_positionIsUpdated;
    bool m_clearTracking;
    bool m_isRecording;

    // n sources
    int m_sources;
    std::vector<uint8> m_ids;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackingVisualizer);
};

//inline bool TrackingVisualizer::isSink()
//{
//    return true;
//}


#endif  // TRACKINGVISUALIZER_H_INCLUDED