/*
 * aec_cmdline.c
 * A straight forward AEC commandline tool based on webrtc
 * by dr-ni <https://github.com/dr-ni>
 */

#include "webrtc/modules/audio_processing/aec/include/echo_cancellation.h"
#include "webrtc/common_audio/signal_processing/include/signal_processing_library.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MONO_FRAME_SIZE 160
#define STEREO_FRAME_SIZE ( MONO_FRAME_SIZE * 2 )
#define SAMPLE_RATE 8000

int main(int argc, char **argv) {

  short input[STEREO_FRAME_SIZE];
  short output[MONO_FRAME_SIZE];
  short rec[MONO_FRAME_SIZE];
  short ref[MONO_FRAME_SIZE];

  size_t input_size = 0;
  size_t output_size = 0;
  int counter = 0;

  void *aecmInst = NULL;
  AecConfig config;
  WebRtcAec_Create(&aecmInst);
  WebRtcAec_Init(aecmInst, 8000, 8000);
  config.nlpMode = kAecNlpConservative;
  WebRtcAec_set_config(aecmInst, config);

  // read the first frame...
  memset(input, 0, sizeof(input));
  memset(output, 0, sizeof(output));
  input_size = fread(input, 1, sizeof(input), stdin);
  do {
    // extract interleaved stereo frame S16_LE to separated mono frames
    // rec and ref
    for (counter = 0; counter < STEREO_FRAME_SIZE; counter += 2) {
      rec[counter / 2] = input[counter];
      ref[counter / 2] = input[counter + 1];
    }

    // do the echo-cancelling
//    speex_echo_cancellation(st, rec, ref, output);
//    speex_preprocess_run(den, output);
      WebRtcAec_BufferFarend(aecmInst, rec, MONO_FRAME_SIZE);
      WebRtcAec_Process(aecmInst, ref, NULL, output, NULL, MONO_FRAME_SIZE, 40, 0);

    //output the processed frame
    output_size = fwrite(output, 1, sizeof(output), stdout);
    if (!output_size) {
      fprintf(stderr, "\nError writing frame\n");
      break;
      // no data writable
    }
    if (input_size < sizeof(input)) {
      break;
      // no data readable
    }
    // Put the next frame in buffer
    memset(input, 0, sizeof(input));
    input_size = fread(input, 1, sizeof(input), stdin);
  } while (input_size == sizeof(input));

  // all done
    WebRtcAec_Free(aecmInst);
  return 0;
}
