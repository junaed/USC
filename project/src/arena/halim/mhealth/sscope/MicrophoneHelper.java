package arena.halim.mhealth.sscope;

import java.io.File;
import java.io.IOException;

import android.app.Activity;
import android.media.MediaRecorder;
import android.os.Environment;
import android.widget.Toast;

public class MicrophoneHelper {

	private MediaRecorder mediaRecorder = null;
	private String fileName;
	private Activity parentActivity;

	public MicrophoneHelper(Activity activity) {
		parentActivity = activity;
	}

	private void startRecording() {

		try {
			fileName = System.currentTimeMillis() + ".3gp";
			mediaRecorder = new MediaRecorder();
			mediaRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
			mediaRecorder.setOutputFormat(MediaRecorder.OutputFormat.THREE_GPP);
			mediaRecorder.setOutputFile(Environment
					.getExternalStorageDirectory()
					+ File.separator
					+ "mhealth"
					+ File.separator + "audio_" + fileName);
			mediaRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AMR_WB);
		} catch (Exception ex) {
			Toast.makeText(parentActivity, ex.getMessage(), Toast.LENGTH_LONG);
		}
		try {
			mediaRecorder.prepare();
		} catch (IllegalStateException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		try {
			mediaRecorder.start();
		} catch (Exception ex) {
			Toast.makeText(parentActivity, ex.getMessage(), Toast.LENGTH_LONG);
		}
	}

	private void stopRecording() {
		mediaRecorder.stop();
		mediaRecorder.release();
		mediaRecorder = null;
	}

	public void onResume() {
		startRecording();
	}

	public void onStop() {
		stopRecording();
	}
}
