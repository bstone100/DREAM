#import "speechrecognizer.h"
#include "QtCore/qdebug.h"
#include "QtCore/qminmax.h"
#import <AVFoundation/AVFoundation.h>
#import <Speech/Speech.h>
#include "audiotranscriptionmanager.h"

@interface SpeechRecognizer : NSObject <SFSpeechRecognizerDelegate>

@property (nonatomic, strong) AVAudioEngine *audioEngine;
@property (nonatomic, strong) SFSpeechRecognizer *speechRecognizer;
@property (nonatomic, strong) SFSpeechAudioBufferRecognitionRequest *recognitionRequest;
@property (nonatomic, strong) SFSpeechRecognitionTask *recognitionTask;
@property (nonatomic, assign) BOOL isCurrentlyRecording;
@property (nonatomic, assign) NSTimeInterval totalDuration;
@property (nonatomic, assign) float audioLevel;

@end

@implementation SpeechRecognizer

- (instancetype)init {
    self = [super init];
    if (self) {
        _audioEngine = [[AVAudioEngine alloc] init];
        _speechRecognizer = [[SFSpeechRecognizer alloc] initWithLocale:[NSLocale localeWithLocaleIdentifier:@"en-US"]];
        _speechRecognizer.delegate = self;
        _isCurrentlyRecording = NO;
        _totalDuration = 0;
        _audioLevel = 0;
    }
    return self;
}

- (void)startRecording {
    if (_isCurrentlyRecording) {
        [self stopRecording];
    }

    [self setupAudioRecorder];
    [self startAudioEngine];
    _isCurrentlyRecording = YES;
}

- (void)stopRecording {
    if (_isCurrentlyRecording) {
        [_audioEngine stop];
        [_audioEngine.inputNode removeTapOnBus:0];
        [_recognitionRequest endAudio];

        if (_recognitionTask) {
            [_recognitionTask cancel];
            _recognitionTask = nil;
        }

        _recognitionRequest = nil;
        _isCurrentlyRecording = NO;
    }
}

- (BOOL)isRecording {
    return _isCurrentlyRecording;
}

- (void)setupAudioRecorder {
#if TARGET_OS_IPHONE
    NSError *error = nil;
    [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryRecord error:&error];
    [[AVAudioSession sharedInstance] setActive:YES error:&error];
    // Handle error if necessary
#endif
}

- (void)startAudioEngine {
    if (_recognitionTask) {
        [_recognitionTask cancel];
        _recognitionTask = nil;
    }

    _recognitionRequest = [[SFSpeechAudioBufferRecognitionRequest alloc] init];
    AVAudioInputNode *inputNode = _audioEngine.inputNode;
    [_recognitionRequest setShouldReportPartialResults:YES];
    _recognitionRequest.requiresOnDeviceRecognition = YES;
    _recognitionRequest.addsPunctuation = YES;

    _recognitionTask = [_speechRecognizer recognitionTaskWithRequest:_recognitionRequest resultHandler:^(SFSpeechRecognitionResult * _Nullable result, NSError * _Nullable error) {
        if (result) {
            SFTranscription *transcription = result.bestTranscription;
            NSString *transcriptionString = transcription.formattedString;
            NSLog(@"Transcription: %@", transcriptionString);

            NSTimeInterval totalDuration = 0;
            for (SFTranscriptionSegment *segment in transcription.segments) {
                totalDuration += segment.duration;
            }

            // detect silence
            if (_totalDuration > totalDuration) {
                dispatch_async(dispatch_get_main_queue(), ^{
                    emit AudioTranscriptionManager::self()->silenceDetected();
                });
            }
            _totalDuration = totalDuration;

            dispatch_async(dispatch_get_main_queue(), ^{
                emit AudioTranscriptionManager::self()->transcriptionUpdated(transcriptionString.UTF8String);
            });
        }
        if (error) {
            [self stopRecording];
            NSLog(@"Error: %@", error.localizedDescription);
        }
    }];

    AVAudioFormat *recordingFormat = [inputNode outputFormatForBus:0];
    [inputNode installTapOnBus:0 bufferSize:1024 format:recordingFormat block:^(AVAudioPCMBuffer * _Nonnull buffer, AVAudioTime * _Nonnull when) {
        [self.recognitionRequest appendAudioPCMBuffer:buffer];
        [self updateAudioLevelFromBuffer:buffer];
    }];
    NSError *audioError = nil;
    [_audioEngine prepare];
    [_audioEngine startAndReturnError:&audioError];
    // Handle error if necessary
}

- (void)updateAudioLevelFromBuffer:(AVAudioPCMBuffer *)buffer {
    float *frame = (float *)buffer.floatChannelData[0];
    float rms = 0;
    for (int i = 0; i < buffer.frameLength; i++) {
        rms += frame[i] * frame[i];
    }
    rms = sqrtf(rms / buffer.frameLength);

    // Normalize RMS to [0, 1]. Adjust 'maxRMS' based on empirical max RMS observed.
    float maxRMS = 0.2;  // This is an example value; you need to calibrate it based on real observations.
    self.audioLevel = MIN(1.0, rms / maxRMS);
}

- (float)getCurrentLevel {
    return _audioLevel;
}

@end

// C functions that bridge to Objective-C implementation
static SpeechRecognizer *sharedRecognizer() {
    static SpeechRecognizer *shared = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        shared = [[SpeechRecognizer alloc] init];
    });
    return shared;
}

void startRecording() {
    [sharedRecognizer() startRecording];
}

void stopRecording() {
    [sharedRecognizer() stopRecording];
}

float getCurrentLevel() {
    return [sharedRecognizer() getCurrentLevel];
}

bool isRecording() {
    return [sharedRecognizer() isRecording];
}
