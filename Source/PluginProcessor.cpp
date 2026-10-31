#include "PluginProcessor.h"
#include "PluginEditor.h"
static std::unique_ptr<juce::RangedAudioParameter> f(const char* id,const char* n,float a,float b,float d,const char* unit=""){
 return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},n,juce::NormalisableRange<float>(a,b),d,unit);
}
BasementAudioProcessor::APVTS::ParameterLayout BasementAudioProcessor::layout(){
 APVTS::ParameterLayout p;
 p.add(f("decay","DECAY",0.1f,12,2.8f," s")); p.add(f("predelay","PRE-DELAY",0,200,24," ms"));
 p.add(f("size","SIZE",0,100,62,"%")); p.add(f("mix","MIX",0,100,25,"%"));
 p.add(f("damping","DAMPING",0,100,58,"%")); p.add(f("width","WIDTH",0,200,115,"%"));
 p.add(f("dirt","DIRT",0,100,32,"%")); p.add(f("lowcut","LOW CUT",20,1000,140," Hz"));
 p.add(f("highcut","HIGH CUT",1000,20000,8500," Hz"));
 p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"freeze",1},"FREEZE",false));
 return p;
}
BasementAudioProcessor::BasementAudioProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),parameters(*this,nullptr,"BASEMENT",layout()){}
void BasementAudioProcessor::prepareToPlay(double rate,int block){
 sr=rate; juce::dsp::ProcessSpec spec{rate,(juce::uint32)block,(juce::uint32)getTotalNumOutputChannels()};
 reverb.prepare(spec); reverb.reset(); predelay.prepare(spec); predelay.setMaximumDelayInSamples((int)(rate*.205)); predelay.reset();
 dry.setSize(getTotalNumOutputChannels(),block);
 auto lo=juce::dsp::IIR::Coefficients<float>::makeHighPass(rate,*parameters.getRawParameterValue("lowcut"));
 auto hi=juce::dsp::IIR::Coefficients<float>::makeLowPass(rate,*parameters.getRawParameterValue("highcut"));
 lowL.coefficients=lo; lowR.coefficients=lo; highL.coefficients=hi; highR.coefficients=hi;
}
void BasementAudioProcessor::releaseResources(){}
void BasementAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&){
 juce::ScopedNoDenormals guard; const int n=b.getNumSamples(),chs=b.getNumChannels(); dry.makeCopyOf(b,true);
 float decay=*parameters.getRawParameterValue("decay"),size=*parameters.getRawParameterValue("size")*.01f;
 float mix=*parameters.getRawParameterValue("mix")*.01f,damp=*parameters.getRawParameterValue("damping")*.01f;
 float width=*parameters.getRawParameterValue("width")*.005f,dirt=*parameters.getRawParameterValue("dirt")*.01f;
 int pd=(int)(sr**parameters.getRawParameterValue("predelay")*.001f); predelay.setDelay((float)juce::jlimit(0,(int)(sr*.2),pd));
 rp.roomSize=juce::jlimit(.08f,.98f,.20f+size*.43f+juce::jlimit(0.0f,1.0f,(decay-.1f)/11.9f)*.34f);
 rp.damping=damp; rp.width=juce::jlimit(0.0f,1.0f,width); rp.wetLevel=1; rp.dryLevel=0;
 rp.freezeMode=*parameters.getRawParameterValue("freeze")>.5f?1.0f:0.0f; reverb.setParameters(rp);
 for(int c=0;c<chs;++c){auto*x=b.getWritePointer(c);for(int i=0;i<n;++i){float drive=1.0f+dirt*5.0f; x[i]=std::tanh(x[i]*drive+(rng.nextFloat()*2-1)*dirt*.012f)/std::tanh(drive); predelay.pushSample(c,x[i]); x[i]=predelay.popSample(c);}}
 juce::dsp::AudioBlock<float> block(b); reverb.process(juce::dsp::ProcessContextReplacing<float>(block));
 for(int c=0;c<chs;++c){auto*w=b.getWritePointer(c);auto*d=dry.getReadPointer(c);auto&lp=c==0?highL:highR;auto&hp=c==0?lowL:lowR;
 for(int i=0;i<n;++i){float v=hp.processSample(lp.processSample(w[i]));v=std::tanh(v*(1+dirt*2.2f))*(1-dirt*.12f);w[i]=d[i]*std::cos(mix*juce::MathConstants<float>::halfPi)+v*std::sin(mix*juce::MathConstants<float>::halfPi);}}
}
void BasementAudioProcessor::loadPreset(int i){
 struct V{float d,p,s,m,da,w,di,lo,hi;}; static constexpr V v[]={{2.8f,24,62,25,58,115,32,140,8500},{1.1f,8,28,22,72,80,45,180,7200},{3.8f,18,55,28,48,105,55,110,6500},{7.5f,42,92,32,62,130,38,90,9500},{2.2f,12,48,30,35,145,68,160,11000}};
 auto a=v[juce::jlimit(0,4,i)]; const char* ids[]={"decay","predelay","size","mix","damping","width","dirt","lowcut","highcut"}; float x[]={a.d,a.p,a.s,a.m,a.da,a.w,a.di,a.lo,a.hi};
 for(int k=0;k<9;++k)if(auto*q=parameters.getParameter(ids[k]))q->setValueNotifyingHost(q->convertTo0to1(x[k])); currentProgram=i;
}
void BasementAudioProcessor::setCurrentProgram(int i){loadPreset(i);}
const juce::String BasementAudioProcessor::getProgramName(int i){const char*n[]={"BASEMENT","GARAGE","BUNKER","ABANDONED FACTORY","DIRTY SPRING"};return n[juce::jlimit(0,4,i)];}
void BasementAudioProcessor::getStateInformation(juce::MemoryBlock& d){std::unique_ptr<juce::XmlElement>x(parameters.copyState().createXml());copyXmlToBinary(*x,d);}
void BasementAudioProcessor::setStateInformation(const void*d,int s){std::unique_ptr<juce::XmlElement>x(getXmlFromBinary(d,s));if(x&&x->hasTagName(parameters.state.getType()))parameters.replaceState(juce::ValueTree::fromXml(*x));}
juce::AudioProcessorEditor* BasementAudioProcessor::createEditor(){return new BasementAudioProcessorEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new BasementAudioProcessor();}
