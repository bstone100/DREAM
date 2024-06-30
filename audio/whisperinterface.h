#ifdef __cplusplus
extern "C" {
#endif

void registerTranscriptionUpdatedSignal(void (*callback)(const char*));
void registerSilenceDetectedSignal(void (*func)(void));
void registerLevelCalculatedSignal(void (*func)(const float));
void registerTimeLimitReachedSignal(void (*func)(void));

bool isCapturing();
float getCurrentLevel();
void setupAudioCapture();
void startAudioCapture();
void stopAudioCapture();

#ifdef __cplusplus
}
#endif
