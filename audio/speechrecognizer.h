// Ensure this interface can be used in C++ files
#ifdef __cplusplus
extern "C" {
#endif

// Public interface for controlling the speech recognizer
void startRecording();
void stopRecording();
float getCurrentLevel();
bool isRecording();

#ifdef __cplusplus
}
#endif
