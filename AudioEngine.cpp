//
// Created by SBNut on 2025-11-04.
//

#include "AudioEngine.hpp"

void AudioEngine::prepare(double sr, int blockSize){
sr_ = sr; block_ = blockSize;
nlms_.prepare(sr_, 256);
stft_.prepare(sr_, 1024);
post_.prepare(sr_);
}


void AudioEngine::process(juce::AudioBuffer<float>& buf, int start, int numSamples){
auto nCh = buf.getNumChannels();
auto *p0 = buf.getWritePointer(0, start);
const float *p1 = nCh>1 ? buf.getReadPointer(1, start) : nullptr;


for (int i=0;i<numSamples;++i){
const float primary = p0[i];
const float reference= p1 ? p1[i] : primary; // fallback
p0[i] = nlms_.processOne(primary, reference);
}


if (usePost_) {
stft_.processChannelInPlace(buf, start, numSamples, 0, post_);
}


if (nCh>1) buf.copyFrom(1, start, buf, 0, start, numSamples);
}
