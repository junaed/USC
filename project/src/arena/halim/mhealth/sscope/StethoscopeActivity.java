package arena.halim.mhealth.sscope;

import android.app.Activity;
import android.content.Context;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class StethoscopeActivity extends Activity {
	/** Called when the activity is first created. */

	Button start_accelerometer, stop_accelerometer, start_mic, stop_mic, button5, button6;
	TextView tv1, tv2;

	private SensorManager sensorManager;
	private AccelerometerHelper accelerometerHelper;
	private MicrophoneHelper microphoneHelper;
	private ExtAudioRecorder extAudioRecorder;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);

		start_accelerometer = (Button) findViewById(R.id.button1);
		stop_accelerometer = (Button) findViewById(R.id.button2);
		start_mic = (Button) findViewById(R.id.button3);
		stop_mic = (Button) findViewById(R.id.button4);
		button5 = (Button) findViewById(R.id.button5);
		button6 = (Button) findViewById(R.id.button6);

		tv1 = (TextView) findViewById(R.id.tv1);
		tv2 = (TextView) findViewById(R.id.tv2);

		stop_accelerometer.setEnabled(false);
		stop_mic.setEnabled(false);
		button6.setEnabled(false);

		sensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);

	}

	public void onBtnClicked(View v) {

		switch (v.getId()) {
		case R.id.button1:
			tv1.setText(R.string.d_stop);
			start_accelerometer.setEnabled(false);
			stop_accelerometer.setEnabled(true);

			accelerometerHelper = new AccelerometerHelper(sensorManager, this);

			accelerometerHelper.onResume();
			break;
		case R.id.button2:
			tv1.setText(R.string.d_start);
			start_accelerometer.setEnabled(true);
			stop_accelerometer.setEnabled(false);

			accelerometerHelper.onStop();
			break;
		case R.id.button3:
			tv2.setText(R.string.d_stop);
			start_mic.setEnabled(false);
			stop_mic.setEnabled(true);

			// microphoneHelper = new MicrophoneHelper(this);
			// microphoneHelper.onResume();

			extAudioRecorder = ExtAudioRecorder.getInstanse(true);
			extAudioRecorder.onResume();

			break;
		case R.id.button4:
			tv2.setText(R.string.d_start);
			start_mic.setEnabled(true);
			stop_mic.setEnabled(false);

			// microphoneHelper.onStop();
			extAudioRecorder.onStop();
			break;
			
		case R.id.button5:
			
			accelerometerHelper = new AccelerometerHelper(sensorManager, this);
			extAudioRecorder = ExtAudioRecorder.getInstanse(true);

			accelerometerHelper.onResume();
			extAudioRecorder.onResume();
			
			start_accelerometer.setEnabled(false);
			stop_accelerometer.setEnabled(true);
			button5.setEnabled(false);
			button6.setEnabled(true);
			
			break;
			
		case R.id.button6:
			
			start_accelerometer.setEnabled(true);
			stop_accelerometer.setEnabled(false);
			button5.setEnabled(true);
			button6.setEnabled(false);

			accelerometerHelper.onStop();
			
			start_mic.setEnabled(true);
			stop_mic.setEnabled(false);

			// microphoneHelper.onStop();
			extAudioRecorder.onStop();
			break;
		default:
			break;
		}
	}

}