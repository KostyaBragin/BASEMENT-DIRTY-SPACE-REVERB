#include "PluginProcessor.h"
#include "PluginEditor.h"
static juce::AudioProcessorValueTreeState::ParameterLayout makeParams(){
 juce::AudioProcessorValueTreeState::ParameterLayout p;
 auto add=[&](const char* id,const char* name,float lo,float hi,float def){p.add(std::make_unique<juce::AudioParameterFloat>(id,name,juce::NormalisableRange<float>(lo,hi,0.01f),def));};
 add("glue","GLUE",0,100,35); add("focus","FOCUS",0,100,55); add("space","SPACE",0,100,20); add("time","TIME",0,100,45); add("mix","MIX",0,100,100); add("output","OUTPUT",-12,12,0);
 return p;
}
SMProductionGlueAudioProcessor::SMProductionGlueAudioProcessor():AudioProcessor(BusesProperties().withInput("Vocal",juce::AudioChannelSet::stereo(),true).withInput("Instrumental Sidechain",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"PARAMETERS",makeParams()){}
bool SMProductionGlueAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const {return l.getMainInputChannelSet()==juce::AudioChannelSet::mono()||l.getMainInputChannelSet()==juce::AudioChannelSet::stereo();}
void SMProductionGlueAudioProcessor::prepareToPlay(double rate,int){sr=rate; envRelease=std::exp(-1.0f/(0.18f*(float)sr)); const float hz[6]={140,350,800,1800,4200,9000}; for(int i=0;i<6;++i){auto c=juce::IIRCoefficients::makeBandPass(sr,hz[i],0.8); bands[i].mainL.setCoefficients(c);bands[i].mainR.setCoefficients(c);bands[i].scL.setCoefficients(c);bands[i].scR.setCoefficients(c);bands[i].mainEnv=bands[i].scEnv=0;bands[i].gain=1;}}
void SMProductionGlueAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&){
 juce::ScopedNoDenormals no; const int n=b.getNumSamples(), ch=b.getNumChannels(); auto* sc=getBusBuffer(b,true,1); const int scCh=sc.getNumChannels();
 const float amount=state.getRawParameterValue("glue")->load()/100.0f, focus=state.getRawParameterValue("focus")->load()/100.0f, space=state.getRawParameterValue("space")->load()/100.0f, time=state.getRawParameterValue("time")->load()/100.0f, wet=state.getRawParameterValue("mix")->load()/100.0f, out=juce::Decibels::decibelsToGain(state.getRawParameterValue("output")->load());
 const float atk=std::exp(-1.0f/((0.008f+time*0.045f)*(float)sr)), rel=std::exp(-1.0f/((0.12f+time*0.55f)*(float)sr));
 for(int i=0;i<n;++i){
  float side[2]={0,0}; if(scCh>0) side[0]=sc.getSample(0,i); if(scCh>1) side[1]=sc.getSample(1,i); else side[1]=side[0];
  float x[2]={b.getSample(0,i),ch>1?b.getSample(1,i):b.getSample(0,i)}, y[2]={x[0],x[1]};
  for(int k=0;k<6;++k){auto& d=bands[k]; float m=0,s=0;
   for(int c=0;c<2;++c){float mv=c==0?d.mainL.processSingleSampleRaw(x[c]):d.mainR.processSingleSampleRaw(x[c]); float sv=c==0?d.scL.processSingleSampleRaw(side[c]):d.scR.processSingleSampleRaw(side[c]); m+=mv*mv*0.5f;s+=sv*sv*0.5f;}
   float me=std::sqrt(m+1e-12f), se=std::sqrt(s+1e-12f); d.mainEnv=std::max(me,d.mainEnv*(me>d.mainEnv?atk:rel)); d.scEnv=std::max(se,d.scEnv*(se>d.scEnv?atk:rel));
   // Psychoacoustic masking model: compare perceptual band energies, then apply only gentle, time-smoothed spectral contouring.
   float ratio=d.scEnv/(d.scEnv+d.mainEnv+1e-5f); float target=1.0f-amount*focus*0.22f*ratio;
   d.gain=0.995f*d.gain+0.005f*target;
   float shaped=1.0f+(d.gain-1.0f)*(0.45f+0.55f*focus);
   y[0]*=shaped; y[1]*=shaped;
  }
  // Spatial integration is deliberately subtle and energy-preserving.
  float mid=(y[0]+y[1])*0.5f, sideSig=(y[0]-y[1])*0.5f; sideSig*=1.0f-space*amount*0.12f; y[0]=mid+sideSig; y[1]=mid-sideSig;
  for(int c=0;c<ch;++c){float v=c==0?y[0]:y[1]; b.setSample(c,i,(x[c]*(1-wet)+v*wet)*out);}
 }
}
juce::AudioProcessorEditor* SMProductionGlueAudioProcessor::createEditor(){return new SMProductionGlueAudioProcessorEditor(*this);}
void SMProductionGlueAudioProcessor::getStateInformation(juce::MemoryBlock& d){auto x=state.copyState();std::unique_ptr<juce::XmlElement> xml(x.createXml());copyXmlToBinary(*xml,d);}
void SMProductionGlueAudioProcessor::setStateInformation(const void* d,int n){std::unique_ptr<juce::XmlElement> x(getXmlFromBinary(d,n));if(x&&x->hasTagName(state.state.getType()))state.replaceState(juce::ValueTree::fromXml(*x));}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new SMProductionGlueAudioProcessor();}