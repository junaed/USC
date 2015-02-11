package arena.halim.mhealth.sscope;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

import android.app.Activity;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.widget.Toast;

public class AccelerometerHelper implements SensorEventListener {

	private final Sensor accelerometer;
	private File logFile;
	private BufferedWriter buf;
	private SensorManager sensorManager;
	private static int DELAY_MODE = SensorManager.SENSOR_DELAY_FASTEST;
	private HandlerThread mHandlerThread;
	private Handler handler;
	private Activity parentActivity;
	private long starttime, endtime;

	private static final boolean ADAPTIVE_ACCEL_FILTER = true;
	private static final boolean HIGH_PASS_FILTER_ON = true;
	double lastAccel[] = new double[3];
	double accelFilter[] = new double[3];

	public AccelerometerHelper(SensorManager sensorManager, Activity activity) {
		// TODO Auto-generated constructor stub

		parentActivity = activity;
		this.sensorManager = sensorManager;
		accelerometer = sensorManager
				.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);

		mHandlerThread = new HandlerThread("sensorThread");
		mHandlerThread.start();
		handler = new Handler(mHandlerThread.getLooper());

	}

	public void onAccuracyChanged(Sensor arg0, int arg1) {
		// TODO Auto-generated method stub

	}

	public void appendLog(String text) {
		try {
			if (buf != null) {
				buf.append(text);
			} else {
				// msg = Toast.makeText(parentActivity, "Buffer is NULL",
				// Toast.LENGTH_LONG);
				// msg.show();
			}

		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	private void updateAccelerometerValue(float x, float y, float z) {

		String text = x + ", " + y + ", " + z ;
		
		if(HIGH_PASS_FILTER_ON)
		{
			
			text += ", " + accelFilter[0] + ", " + accelFilter[1] + ", " + accelFilter[2];
		}
		
		text += "\n";
		// appendLog(text);

		try {
			if (buf != null) {
				buf.append(text);
			} else {
				// msg = Toast.makeText(parentActivity, "Buffer is NULL",
				// Toast.LENGTH_LONG);
				// msg.show();
			}

		} catch (IOException e) {
			e.printStackTrace();
		}

		// System.out.println(text);
	}

	private double norm(double x, double y, double z) {
		return Math.sqrt(x * x + y * y + z * z);
	}

	private double clamp(double v, double min, double max) {
		if (v > max)
			return max;
		else if (v < min)
			return min;
		else
			return v;
	}

	public void onSensorChanged(SensorEvent arg0) {
		// TODO Auto-generated method stub
		// System.out.println("In");
		if (arg0.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {

			// high pass filter
			if (HIGH_PASS_FILTER_ON) {
				float updateFreq = 30; // match this to your update speed
				float cutOffFreq = 0.9f;
				float RC = 1.0f / cutOffFreq;
				float dt = 1.0f / updateFreq;
				float filterConstant = RC / (dt + RC);
				double alpha = filterConstant;
				float kAccelerometerMinStep = 0.033f;
				float kAccelerometerNoiseAttenuation = 3.0f;

				if (ADAPTIVE_ACCEL_FILTER) {
					double d = clamp(
							Math.abs(norm(accelFilter[0], accelFilter[1],
									accelFilter[2])
									- norm(arg0.values[0], arg0.values[1],
											arg0.values[2]))
									/ kAccelerometerMinStep - 1.0f, 0.0f, 1.0f);
					alpha = d * filterConstant / kAccelerometerNoiseAttenuation
							+ (1.0f - d) * filterConstant;
				}

				accelFilter[0] = (float) (alpha * (accelFilter[0]
						+ arg0.values[0] - lastAccel[0]));
				accelFilter[1] = (float) (alpha * (accelFilter[1]
						+ arg0.values[1] - lastAccel[1]));
				accelFilter[2] = (float) (alpha * (accelFilter[2]
						+ arg0.values[2] - lastAccel[2]));

				lastAccel[0] = arg0.values[0];
				lastAccel[1] = arg0.values[1];
				lastAccel[2] = arg0.values[2];
			}
			updateAccelerometerValue(arg0.values[0], arg0.values[1],
					arg0.values[2]);
		}

	}

	public Sensor getAccelerometer() {
		return accelerometer;
	}

	private void createFile() {

		String fname = System.currentTimeMillis() + ".csv";
		// logFile = new File("/sdcard/sensor_reading_" + fname);

		String state = Environment.getExternalStorageState();

		if (Environment.MEDIA_MOUNTED.equalsIgnoreCase(state)) {

			System.out.println("SD card found");
			logFile = new File(Environment.getExternalStorageDirectory()
					+ File.separator + "mhealth" + File.separator
					+ "accelerometer_" + fname);
			if (!logFile.exists()) {
				try {
					logFile.createNewFile();
				} catch (IOException e) {
					e.printStackTrace();
					Toast.makeText(parentActivity, e.getMessage(),
							Toast.LENGTH_LONG);
				}
			}
		}

		else {
			System.out.println("No SD card");
			logFile = new File(Environment.getRootDirectory() + File.separator
					+ "mhealth_" + "accelerometer_" + fname);
		}
		try {
			buf = new BufferedWriter(new FileWriter(logFile, true));
		} catch (IOException e) {
			Toast.makeText(parentActivity, e.toString(), Toast.LENGTH_LONG)
					.show();
		}

	}

	public void onResume() {
		createFile();

		starttime = System.currentTimeMillis();
		sensorManager.registerListener(this, accelerometer,
				AccelerometerHelper.DELAY_MODE, handler);
	}

	public void onStop() {
		sensorManager.unregisterListener(this, accelerometer);
		endtime = System.currentTimeMillis();

		long ttime = endtime - starttime;
		appendLog(ttime + "");

		try {
			if (buf != null) {
				buf.flush();
				buf.close();
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}

}
