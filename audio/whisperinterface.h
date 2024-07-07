#ifdef __cplusplus
extern "C" {
#endif

bool isCapturing();
float getCurrentLevel();
void setupAudioCapture();
void startAudioCapture();
void stopAudioCapture();

#ifdef __cplusplus
}
#endif
