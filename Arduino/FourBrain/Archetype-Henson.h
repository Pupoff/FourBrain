#pragma once


// Function to initialize parameters with values and MIDI notes
void initializeParameters() {
  static const char* const ampLabels[3] = { "Clean", "Crunch", "Lead" };

  // AMP SECTION        //MIDI from 10 to 37 (20-27=crunch, 30-37=lead)
  parameters[AMP][0] = (Parameter){ 1, 0, SWITCH, "AMP", nullptr };               // first field is for enable, the label correspond to the section name
  parameters[AMP][1] = (Parameter){ 30, 1, KNOB, "Gain", nullptr };               // cleanGain
  parameters[AMP][2] = (Parameter){ 10, 2, KNOB, "Bass", nullptr };               // cleanBass
  parameters[AMP][3] = (Parameter){ 50, 3, KNOB, "Mid", nullptr };                // cleanM
  parameters[AMP][4] = (Parameter){ 70, 4, KNOB, "Treble", nullptr };             // cleanTreble
  parameters[AMP][5] = (Parameter){ 60, 5, KNOB, "Presence", nullptr };           // cleanPresence
  parameters[AMP][6] = (Parameter){ 40, 6, KNOB, "Blend", nullptr };              // cleanBlend
  parameters[AMP][7] = (Parameter){ 40, 7, KNOB, "Output", nullptr };             // cleanMaster
  parameters[AMP][8] = (Parameter){ 1, 53, THREE_WAY_SWITCH, "Amp", ampLabels };  //Yes, this is in AMP (and yes parameters_number[AMP] = 7 despite it being at idx 8)

  parameters_number[AMP] = 8;

  parameters[AMP][11] = (Parameter){ 30, 8, KNOB, "Gain", nullptr };       // crunchGain
  parameters[AMP][12] = (Parameter){ 10, 9, KNOB, "Bass", nullptr };       // crunchBass
  parameters[AMP][13] = (Parameter){ 50, 10, KNOB, "Mid", nullptr };       // crunchM
  parameters[AMP][14] = (Parameter){ 70, 11, KNOB, "Treble", nullptr };    // crunchTreble
  parameters[AMP][15] = (Parameter){ 60, 12, KNOB, "Presence", nullptr };  // crunchPresence
  parameters[AMP][16] = (Parameter){ 1, 13, SWITCH, "Channel", nullptr };
  parameters[AMP][17] = (Parameter){ 40, 14, KNOB, "Output", nullptr };  // crunchMaster

  parameters[AMP][21] = (Parameter){ 30, 15, KNOB, "Gain", nullptr };      // LeadGain
  parameters[AMP][22] = (Parameter){ 10, 16, KNOB, "Bass", nullptr };      // LeadBass
  parameters[AMP][23] = (Parameter){ 50, 17, KNOB, "Mid", nullptr };       // LeadM
  parameters[AMP][24] = (Parameter){ 70, 18, KNOB, "Treble", nullptr };    // LeadTreble
  parameters[AMP][25] = (Parameter){ 60, 19, KNOB, "Presence", nullptr };  // LeadPresence
  parameters[AMP][26] = (Parameter){ 40, 20, KNOB, "Blend", nullptr };     // LeadBlend
  parameters[AMP][27] = (Parameter){ 40, 21, KNOB, "Output", nullptr };    // LeadMaster

  // DELAY
  static const char* const delayModeLabels[3] = { "Normal", "Wide", "PingPong" };
  static const char* const delayTypeLabels[3] = { "Diffusion", "Digital", "Modern" };
  static const char* const delaySyncLabels[3] = { "Tap", "Daw", "Free" };

  parameters[DELAY][0] = (Parameter){ 1, 22, SWITCH, "DELAY", nullptr };
  parameters[DELAY][1] = (Parameter){ 10, 23, KNOB, "Mix", nullptr };
  parameters[DELAY][2] = (Parameter){ 20, 24, KNOB, "Feedback", nullptr };
  parameters[DELAY][3] = (Parameter){ 70, 25, KNOB, "Low-Cut", nullptr };
  parameters[DELAY][4] = (Parameter){ 50, 26, KNOB, "High-Cut", nullptr };
  parameters[DELAY][5] = (Parameter){ 60, 27, KNOB, "Time", nullptr };
  parameters[DELAY][6] = (Parameter){ 70, 28, KNOB, "Amount", nullptr };
  parameters[DELAY][7] = (Parameter){ 1, 29, THREE_WAY_SWITCH, "Sync", delaySyncLabels };
  parameters[DELAY][8] = (Parameter){ 1, 30, THREE_WAY_SWITCH, "Mode", delayModeLabels };
  parameters[DELAY][9] = (Parameter){ 1, 31, THREE_WAY_SWITCH, "Type", delayTypeLabels };
  parameters_number[DELAY] = 9;

  // COMPRESSOR
  parameters[COMPRESSOR][0] = (Parameter){ 1, 32, SWITCH, "COMP.", nullptr };
  parameters[COMPRESSOR][1] = (Parameter){ 70, 33, KNOB, "Level", nullptr };      // compressorPedalLevel
  parameters[COMPRESSOR][2] = (Parameter){ 70, 34, KNOB, "Threshold", nullptr };  // compressorPedalThres
  parameters[COMPRESSOR][3] = (Parameter){ 1, 35, SWITCH, "Attack ", nullptr };   //Attack: slow/fast
  parameters_number[COMPRESSOR] = 3;

  // OVERDRIVE
  parameters[OVERDRIVE][0] = (Parameter){ 1, 36, SWITCH, "OVERDRIVE", nullptr };
  parameters[OVERDRIVE][1] = (Parameter){ 70, 37, KNOB, "Gain", nullptr };   //overDrivePedalGain
  parameters[OVERDRIVE][2] = (Parameter){ 70, 38, KNOB, "Tone", nullptr };   // overDrivePedalTone
  parameters[OVERDRIVE][3] = (Parameter){ 70, 39, KNOB, "Level", nullptr };  // overDrivePedalLevel
  parameters_number[OVERDRIVE] = 3;



  // BOOST
  parameters[BOOST][0] = (Parameter){ 1, 40, SWITCH, "BOOST", nullptr };
  parameters[BOOST][1] = (Parameter){ 70, 41, KNOB, "Gain", nullptr };
  parameters[BOOST][2] = (Parameter){ 70, 42, KNOB, "Level", nullptr };
  parameters[BOOST][3] = (Parameter){ 70, 43, KNOB, "Bass", nullptr };
  parameters[BOOST][4] = (Parameter){ 70, 44, KNOB, "Treeble", nullptr };
  parameters_number[BOOST] = 4;

  //REVERB
  parameters[REVERB][0] = (Parameter){ 1, 45, SWITCH, "REVERB", nullptr };
  parameters[REVERB][1] = (Parameter){ 70, 46, KNOB, "Mix", nullptr };
  parameters[REVERB][2] = (Parameter){ 70, 47, KNOB, "Decay", nullptr };
  parameters[REVERB][3] = (Parameter){ 70, 48, KNOB, "LowCut", nullptr };
  parameters[REVERB][4] = (Parameter){ 70, 49, KNOB, "HighCut", nullptr };
  parameters[REVERB][5] = (Parameter){ 1, 50, SWITCH, "Shimmer ", nullptr };
  parameters_number[REVERB] = 5;

  //CHORUS
  parameters[CHORUS][0] = (Parameter){ 1, 51, SWITCH, "CHORUS", nullptr };
  parameters[CHORUS][1] = (Parameter){ 70, 52, KNOB, "Mix", nullptr };
  parameters_number[CHORUS] = 1;


  // -----------------------------------------------------------------------------
  // SPECIAL "OTHER" PARAMETERS
  // These parameters are NOT swept automatically like the other sections.
  // They are accessed explicitly in the code.
  //
  // IMPORTANT:
  // - Change OtherParamId struct (in midicortex.h), not their index directly
  // - Each parameter MUST have a unique assigned index.
  // - Access them using the dedicated enum (e.g. OTHER_ON_OFF)
  // - Don't forget an index
  // - You can reassign an index: e.g. if you don't used OTHER_DOUBLER, change it here and every call in the code
  // -----------------------------------------------------------------------------

  //53 is taken by amp selection!!!
  parameters[OTHER][OTHER_INPUT_GAIN] = (Parameter){ 70, 54, KNOB, "inputGain", nullptr };    //inputGain
  parameters[OTHER][OTHER_OUTPUT_GAIN] = (Parameter){ 70, 55, KNOB, "outputGain", nullptr };  //outputGain
  parameters[OTHER][OTHER_DOUBLER] = (Parameter){ 0, 56, SWITCH, "doubler", nullptr };
  parameters[OTHER][OTHER_PARAM_OPEN] = (Parameter){ 1, 57, SWITCH, "param_open", nullptr };  //custom
  parameters[OTHER][OTHER_ON_OFF] = (Parameter){ 1, 58, SWITCH, "on_off", nullptr };          //custom
  // parameters[OTHER][OTHER_PRESET_UP] = (Parameter){ 1,  59, SWITCH, "NC" , nullptr};         //Just here for the midi note, other agruments not used
  // parameters[OTHER][OTHER_PRESET_DOWN] = (Parameter){ 1,  60, SWITCH, "NC" , nullptr};         //Just here for the midi note, other agruments not used

  //NOT USED
  //  parameters[OTHER][5] = (Parameter){ 1,  63, SWITCH, "Effect" , nullptr};         //Effect activ
  //  parameters[OTHER][6] = (Parameter){ 1,  64, SWITCH, "Time" , nullptr};           //Time activ
  //  parameters[OTHER][7] = (Parameter){ 70, 65, KNOB, "quality" , nullptr};         //quality
  //  parameters[OTHER][8] = (Parameter){ 70, 66, KNOB, "gate" , nullptr};            //gate
  //  parameters[OTHER][9] = (Parameter){ 70, 67, SWITCH, "mode" , nullptr};          //mode


  parameters_number[OTHER] = 7;

  // EQ SECTION (9-band EQ) MIDI NOT USED, to configure
  parameters[EQ][0] = (Parameter){ 1, 0, SWITCH, "EQ", nullptr };  // Set all EQ bands to 0.5, with MIDI notes 72-81 NOT a 3 way switch as it is always the same as the AMP type
                                                                   // parameters[EQ_CRUNCH][0] = (Parameter){0, 80,SWITCH,"Eq Crunch"};  //
                                                                   // parameters[EQ_LEAD][0] = (Parameter){0, 90,SWITCH, "Eq Lead"};  //
  for (int i = 1; i < 10; i++) {
    parameters[EQ][i] = (Parameter){ 50, 0 + i, SLIDER, "Eq", nullptr };  // Set all EQ bands to 0.5, with MIDI notes 72-81
    //  parameters[EQ_CRUNCH][i] = (Parameter){50, 81 + i,KNOB,"Eq"};  //
    //  parameters[EQ_LEAD][i] = (Parameter){50, 91 + i,KNOB, "Eq"};  //
  }
  parameters_number[EQ] = 9;
}

