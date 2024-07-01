#include "whisperinterface.h"
#include <dispatch/dispatch.h>
#include "../whisper.cpp/whisper.h"
#include "QDebug"
#include "audiotranscriptionmanager.h"
#include "../mainwindow.h"
#include "QElapsedTimer"
#include "../widgets/audiorecorderwidget.h"

#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioQueue.h>

#define NUM_BUFFERS 3
#define MAX_AUDIO_SEC 30
#define SAMPLE_RATE 16000

#define NUM_BYTES_PER_BUFFER 16*1024

struct StateInp {
    int ggwaveId;
    bool isCapturing;
    bool isTranscribing;
    bool isRealtime;

    AudioQueueRef queue;
    AudioStreamBasicDescription dataFormat;
    AudioQueueBufferRef buffers[NUM_BUFFERS];

    int n_samples;
    int16_t * audioBufferI16;
    float   * audioBufferF32;

    struct whisper_context * ctx;

    // VAD
    bool isSpeechStarted;
    float silenceTimeOut; // in seconds
    float currentSilenceDuration;

    float noiseEstimate;           // Estimated noise level
    float adaptiveThreshold;       // Dynamic threshold based on noise estimate
    float thresholdMin;            // Minimum threshold to avoid too low sensitivity
    float thresholdMax;
    float thresholdFactor;         // Factor to multiply noise estimate to set threshold
};

static StateInp stateInp;

static QElapsedTimer stopwatch;

// Callback function declaration for handling audio input
static void AudioInputCallback(void *userData, AudioQueueRef queue, AudioQueueBufferRef buffer, const AudioTimeStamp *startTime, UInt32 numPacketDescriptions, const AudioStreamPacketDescription *packetDescs);

void setupAudioCapture() {
    // whisper.cpp initialization
    // load the model
    NSString *modelPath = [[NSBundle mainBundle] pathForResource:@"ggml-base.en" ofType:@"bin"];

    // check if the model exists
    if (![[NSFileManager defaultManager] fileExistsAtPath:modelPath]) {
      NSLog(@"Model file not found");
      return;
    }

    NSLog(@"Loading model from %@", modelPath);

    // create ggml context

    struct whisper_context_params cparams = whisper_context_default_params();
    #if TARGET_OS_SIMULATOR
    cparams.use_gpu = false;
    NSLog(@"Running on simulator, using CPU");
    #endif
    stateInp.ctx = whisper_init_from_file_with_params([modelPath UTF8String], cparams);

    // check if the model was loaded successfully
    if (stateInp.ctx == NULL) {
      NSLog(@"Failed to load model");
      return;
    }

    // initialize audio format and buffers

    stateInp.dataFormat.mSampleRate       = WHISPER_SAMPLE_RATE;
    stateInp.dataFormat.mFormatID         = kAudioFormatLinearPCM;
    stateInp.dataFormat.mFramesPerPacket  = 1;
    stateInp.dataFormat.mChannelsPerFrame = 1;
    stateInp.dataFormat.mBytesPerFrame    = 2;
    stateInp.dataFormat.mBytesPerPacket   = 2;
    stateInp.dataFormat.mBitsPerChannel   = 16;
    stateInp.dataFormat.mReserved         = 0;
    stateInp.dataFormat.mFormatFlags      = kLinearPCMFormatFlagIsSignedInteger;


    stateInp.n_samples = 0;
    stateInp.audioBufferI16 = (int16_t *)malloc(MAX_AUDIO_SEC*SAMPLE_RATE*sizeof(int16_t));
    stateInp.audioBufferF32 = (float *)malloc(MAX_AUDIO_SEC*SAMPLE_RATE*sizeof(float));

    stateInp.isTranscribing = false;
    stateInp.isRealtime = true;

    stateInp.silenceTimeOut = 1.0; // seconds of silence before stopping

    stateInp.thresholdMin = 0.01;      // Prevents the threshold from becoming too low
    stateInp.thresholdMax = 0.1;
    stateInp.thresholdFactor = 4.0;    // Example scaling factor
}

// called from main thread
void startAudioCapture() {
    // initiate audio capturing
    NSLog(@"Start capturing");

#if TARGET_OS_IPHONE
    NSError *error = nil;

    // Configure the audio session for recording
    AVAudioSession *session = [AVAudioSession sharedInstance];
    [session setCategory:AVAudioSessionCategoryRecord error:&error];
    [session setMode:AVAudioSessionModeDefault error:&error];
    [session setActive:YES withOptions:0 error:&error];

    if (error) {
        NSLog(@"Error configuring audio session: %@", error);
        return;
    }
#endif

    stateInp.n_samples = 0;
    stateInp.currentSilenceDuration = 0.0;

    stateInp.noiseEstimate = 0.0;
    stateInp.adaptiveThreshold = 0.1;

    stateInp.isSpeechStarted = false;

    OSStatus status = AudioQueueNewInput(&stateInp.dataFormat,
                                         AudioInputCallback,
                                         &stateInp,
                                         CFRunLoopGetCurrent(),
                                         kCFRunLoopCommonModes,
                                         0,
                                         &stateInp.queue);

    // Enable metering
    UInt32 enableMetering = 1; // true
    AudioQueueSetProperty(stateInp.queue, kAudioQueueProperty_EnableLevelMetering, &enableMetering, sizeof(enableMetering));


    if (status == 0) {
        for (int i = 0; i < NUM_BUFFERS; i++) {
            AudioQueueAllocateBuffer(stateInp.queue, NUM_BYTES_PER_BUFFER, &stateInp.buffers[i]);
            AudioQueueEnqueueBuffer (stateInp.queue, stateInp.buffers[i], 0, NULL);
        }

        // audio capture happens in background thread
        stateInp.isCapturing = true;
        status = AudioQueueStart(stateInp.queue, NULL);
        if (status == 0) {
            // we're capturing
        }
    }

    if (status != 0) {
        stopAudioCapture();
    }
}

// called from main thread
void stopAudioCapture() {
    // don't block main thread
    // dispatch to background thread
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSLog(@"Stop capturing");

        stateInp.isCapturing = false;

        AudioQueueStop(stateInp.queue, true);
        for (int i = 0; i < NUM_BUFFERS; i++) {
            AudioQueueFreeBuffer(stateInp.queue, stateInp.buffers[i]);
        }

        AudioQueueDispose(stateInp.queue, true);
    });
}

void clearAudioBuffers() {
    NSLog(@"Clearing buffer contents");

    // Reset the sample counter
    stateInp.n_samples = 0;

    // Clear each buffer
    for (int i = 0; i < NUM_BUFFERS; i++) {
        memset(stateInp.buffers[i]->mAudioData, 0, stateInp.buffers[i]->mAudioDataBytesCapacity);
        stateInp.buffers[i]->mAudioDataByteSize = 0;
    }

    NSLog(@"Buffers cleared");
}

bool isCapturing() {
    return stateInp.isCapturing;
}

float getCurrentLevel() {
    if (!stateInp.isCapturing) {
        return 0.0;
    }

    UInt32 dataSize = sizeof(AudioQueueLevelMeterState);
    AudioQueueLevelMeterState *levels = (AudioQueueLevelMeterState *)malloc(dataSize);

    if (!levels) {
        NSLog(@"Memory allocation failed for AudioQueueLevelMeterState");
        return 0.0f; // Handle memory allocation failure
    }

    // Get the metering data
    OSStatus status = AudioQueueGetProperty(stateInp.queue, kAudioQueueProperty_CurrentLevelMeter, levels, &dataSize);
    if (status != noErr) {
        NSLog(@"Failed to retrieve level meter property");
        free(levels);
        return 0.0f;
    }

    // Assuming mono audio or getting only the first channel level
    float level = levels[0].mAveragePower;
    level = qBound(0.0, level * 3, 1.0); // make it bigger (personal preference)

    free(levels);
    return level;
}




void onTranscribe() {
    if (stateInp.isTranscribing) {
        return;
    }

    if (stopwatch.isValid()) {
        qDebug() << "Elapsed: " << stopwatch.elapsed();
    }
    stopwatch.start();

    NSLog(@"Processing %d samples", stateInp.n_samples);

    stateInp.isTranscribing = true;

    // dispatch the model to a background thread
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        // process captured audio
        // convert I16 to F32
        for (int i = 0; i < stateInp.n_samples; i++) {
            stateInp.audioBufferF32[i] = (float)stateInp.audioBufferI16[i] / 32768.0f;
        }

        // run the model
        struct whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);

        // get maximum number of threads on this device (max 8)
        const int max_threads = MIN(8, (int)[[NSProcessInfo processInfo] processorCount]);

        params.print_realtime   = false;
        params.print_progress   = false;
        params.print_timestamps = false;
        params.print_special    = false;
        params.translate        = false;
//        params.language         = "en";
        params.language = nullptr;
        params.n_threads        = max_threads;
        params.offset_ms        = 0;
        params.no_context       = true;
        params.single_segment   = stateInp.isRealtime;
        params.no_timestamps    = params.single_segment;
        params.suppress_blank   = true;
        params.suppress_non_speech_tokens = true;

        auto startTime = clock();

        whisper_reset_timings(stateInp.ctx);

        if (whisper_full(stateInp.ctx, params, stateInp.audioBufferF32, stateInp.n_samples) != 0) {
            NSLog(@"Failed to run the model");
            return;
        }

//        whisper_print_timings(stateInp.ctx);

        auto endTime = clock();

        NSLog(@"\nProcessing time: %5.3lu, on %d threads", endTime - startTime, params.n_threads);

        // result text
        NSString *result = @"";

        int n_segments = whisper_full_n_segments(stateInp.ctx);
        for (int i = 0; i < n_segments; i++) {
            const char * text_cur = whisper_full_get_segment_text(stateInp.ctx, i);

            // append the text to the result
            result = [result stringByAppendingString:[NSString stringWithUTF8String:text_cur]];
        }

        // dispatch the result to the main thread
        dispatch_async(dispatch_get_main_queue(), ^{
            stateInp.isTranscribing = false;
            emit AudioTranscriptionManager::self()->transcriptionUpdated(result.UTF8String);
        });
    });
}


// called from background thread
void AudioInputCallback(void * inUserData,
                        AudioQueueRef inAQ,
                        AudioQueueBufferRef inBuffer,
                        const AudioTimeStamp * inStartTime,
                        UInt32 inNumberPacketDescriptions,
                        const AudioStreamPacketDescription * inPacketDescs)
{
    StateInp * stateInp = (StateInp*)inUserData;

    if (!stateInp->isCapturing) {
        NSLog(@"Not capturing, ignoring audio");
        return;
    }

    const int n = inBuffer->mAudioDataByteSize / 2;

    NSLog(@"Captured %d new samples", n);

    // for now just stop recording
    // main window will call stop function
    if (stateInp->n_samples + n > MAX_AUDIO_SEC*SAMPLE_RATE) {
        NSLog(@"Audio recording time limit reached.");

        dispatch_async(dispatch_get_main_queue(), ^{
            // main window will begin appending onto current transcription instead of replacing
            emit AudioTranscriptionManager::self()->timeLimitReached();
        });
        return;
    }

    // copy the data
    for (int i = 0; i < n; i++) {
        stateInp->audioBufferI16[stateInp->n_samples + i] = ((short*)inBuffer->mAudioData)[i];
    }

    // put the buffer back in the queue
    AudioQueueEnqueueBuffer(stateInp->queue, inBuffer, 0, NULL);

    float currentLevel = getCurrentLevel();

    // Noise estimation and adaptive thresholding
    if (currentLevel < stateInp->adaptiveThreshold) {
        stateInp->noiseEstimate = 0.5 * stateInp->noiseEstimate + 0.5 * currentLevel; // Update noise estimate with low-pass filter
        stateInp->adaptiveThreshold = qBound(stateInp->thresholdMin, stateInp->noiseEstimate * stateInp->thresholdFactor, stateInp->thresholdMax);
    }

    NSLog(@"Current Level: %f, Noise Estimate: %f, Adaptive Threshold: %f", currentLevel, stateInp->noiseEstimate, stateInp->adaptiveThreshold);


    stateInp->n_samples += n;

    // Update silence detection
    if (currentLevel < stateInp->adaptiveThreshold) {
        stateInp->currentSilenceDuration += ((float)n / stateInp->dataFormat.mSampleRate);
    } else {
        stateInp->currentSilenceDuration = 0;
        stateInp->isSpeechStarted = true;
    }

    NSLog(@"Current silence duration: %f", stateInp->currentSilenceDuration);

    // Check if the user stopped talking
    // if so then clear the buffers but keep recording
    bool speechDetected = stateInp->isSpeechStarted || AudioRecorderWidget::self()->getCurrentTranscriptionWordCount() > 1;
    if (stateInp->currentSilenceDuration >= stateInp->silenceTimeOut && speechDetected) {
        NSLog(@"Silence detected.");

        // notify UI
        dispatch_async(dispatch_get_main_queue(), ^{
            emit AudioTranscriptionManager::self()->silenceDetected();
        });

        // clear buffers, keep recording
        stateInp->isSpeechStarted = false;
        stateInp->currentSilenceDuration = 0.0;
        clearAudioBuffers();

        return;
    }

    if (stateInp->isRealtime) {
        // dipatch onTranscribe() to the main thread
        dispatch_async(dispatch_get_main_queue(), ^{
            onTranscribe();
        });
    }
}










