# Import libraries
import datetime
import numpy as np
import paho.mqtt.client as client
import math
import psycopg2
import threading

# Arrays to store threads and acceleration data

x = []
y = []
z = []
freque_x = []
freque_y = []
freque_z = []
amplit_x = []
amplit_y = []
amplit_z = []
DB_threads_x = []
DB_threads_y = []
DB_threads_z = []
DB_peaks_x = []
DB_peaks_y = []
DB_peaks_z = []
location = "warehouse1"
sensor_no = 1



hostname = "172.20.10.3"
topic = "warehouse1/sensor1/#"  # Subscribe to 3 topics at the same time
topic_vel_x = 'accel_data/sensor1/velocity_x'
topic_vel_y = 'accel_data/sensor1/velocity_y'
topic_vel_z = 'accel_data/sensor1/velocity_z'
topic_cat_x = 'accel_data/sensor1/vel_categ_x'
topic_cat_y = 'accel_data/sensor1/vel_categ_y'
topic_cat_z = 'accel_data/sensor1/vel_categ_z'
topic_peak_f_x = 'accel_data/sensor1/peaks_f_x'
topic_peak_a_x = 'accel_data/sensor1/peaks_a_x'
topic_peak_f_y = 'accel_data/sensor1/peaks_f_y'
topic_peak_a_y = 'accel_data/sensor1/peaks_a_y'
topic_peak_f_z = 'accel_data/sensor1/peaks_f_z'
topic_peak_a_z = 'accel_data/sensor1/peaks_a_z'
topic_local = 'accel_data/sensor1/location'

# FFT variables
samples = 1024
half_samples = int(samples / 2)
sample_freq = 1600

# Connect to Postgres
try:
    connection = psycopg2.connect(user='pgadmin',
                                  password='Thursday123',
                                  host='172.20.10.3',
                                  port='5432',
                                  database='IoTsystem')
    print("Connected")

except (Exception, psycopg2.Error) as error:
    print("Error while connecting to PostgreSQL", error)

cursor = connection.cursor()

# Callback function for x, y, z axis data to get added to the array whenever data arrives via MQTT
def callback_x(client, userdata, msg):
    msg = msg.payload.decode()
    x.append(float(msg))
    print(x)


def callback_y(client, userdata, msg):
    msg = msg.payload.decode()
    y.append(float(msg))


def callback_z(client, userdata, msg):
    msg = msg.payload.decode()
    z.append(float(msg))


def on_connect(client, userdata, flags, rc):
    print("Connection returned result: " + str(rc))
    client.subscribe(topic)


# FFT function
def fft_blah(accel, fs, Num):
    # Perform FFT
    x_fft = np.fft.fft(accel, Num)  # FFT of the first sample
    x_mag = (np.abs(x_fft)) / Num

    # Create frequency spectrum
    f_step = fs / Num  # Each frequency bin is f_step
    f = np.linspace(0, (Num - 1) * f_step, Num)  # Plot the frequency spectrum
    f_plot = f[1:int(Num / 2 + 1)]
    x_mag_plot = 2 * x_mag[1:int(Num / 2 + 1)]
    # It goes from sample 2 because first sample is DC component and that one should not be multiplied

    return f_plot, np.round(x_mag_plot, 3)


def velocity(freq, amp, half_fft):
    # Zero pad an array of 512
    a_f = []
    amp_rms = []
    for i in range(512):
        a_f.append(0)
        amp_rms.append(0)

    # Calculate velocity
    for i in range(1,512):
        # Each amplitude and frequency are divided from each other and squared
        amp_rms[i] = amp[i]*(1/np.sqrt(2))
        sample = (amp_rms[i] / freq[i])**2
        a_f[i] = sample

    # All of these squares are then summed and multiplied by 10**3/pi - tadaaa velocity of each samples is done
    vel = (1000/(2*math.pi))*(math.sqrt(sum(a_f)))

    return round(vel, 3)


def find_peaks(freq, amp):
    box = []
    indices = []

    # We enumerate/iterate the list and we put the higher value on top by reversing it
    ranks = sorted([(x, i) for (i, x) in enumerate(amp)], reverse=True)

    # We need a box to store the highest values in, so we loop through the list and if the value
    # Is not there yet, we put it there, we start from the beginning because the values are anyways
    # Sorted in descending order so we only take the first 5 highest values, after 5 values we break
    for x, i in ranks:
        if x not in box:
            box.append(x)
            indices.append(i)

            if len(box) == 3:
                break
    return indices, box

# Compare velocity result to the ISO standard table and evaluate which category it falls into
def category(veloc_value):
    if veloc_value <= 0.71:
        return 'A'
    if (veloc_value > 0.71) & (veloc_value <= 1.8):
        return 'B'
    if (veloc_value > 1.8) & (veloc_value <= 4.5):
        return 'C'
    else:
        return 'D'

# While there are not 1024 samples yet, continue waiting
def wait_for_data():
    while len(x) & len(y) & len(z) < samples:
        pass

# Insert sensor info if it is not in the database yet
def DB_insert_sensor_info(ID, local):
    client.publish(topic_local, ('l' + local))
    get_sensorID = """select * from sensor_id where sensor_id = %s"""
    cursor.execute(get_sensorID, (ID,))
    sensor = cursor.fetchall()

    if sensor != '':
        if sensor == []:
            postgres_insert = """ INSERT INTO sensor(sensor_id, location) VALUES(%s, %s)"""
            cursor.execute(postgres_insert, (ID, local))
            connection.commit()

# Insert FFT - not used, maybe to be used in the future
def DB_insert_FFT(ID, f, a, time_stamp, axis):
    if axis == "x":
        postgres_insert = """ INSERT INTO fft_x(sensor_id, fft, frequency, time_stamp) VALUES(%s, %s, %s, %s)"""
        cursor.execute(postgres_insert, (ID, a, f, time_stamp))
        # print(cursor.rowcount, "Record inserted succesfully into Velocity table")
        connection.commit()

    if axis == "y":
        postgres_insert = """ INSERT INTO fft_y(sensor_id, fft, frequency, time_stamp) VALUES(%s, %s, %s, %s)"""
        cursor.execute(postgres_insert, (ID, a, f, time_stamp))
        # print(cursor.rowcount, "Record inserted succesfully into Velocity table")
        connection.commit()

    if axis == "z":
        postgres_insert = """ INSERT INTO fft_z(sensor_id, fft, frequency, time_stamp) VALUES(%s, %s, %s, %s)"""
        cursor.execute(postgres_insert, (ID, a, f, time_stamp))
        # print(cursor.rowcount, "Record inserted succesfully into Velocity table")
        connection.commit()

# Insert velocity into Postgres DB
def DB_insert_velocity(ID, v, cat, time_stamp, axis):
    if axis == "x":
        postgres_insert = """ INSERT INTO VELOCITY_X(sensor_id, velocity, category, time_stamp) VALUES(%s, %s, %s, %s)"""
        cursor.execute(postgres_insert, (ID, v, cat, time_stamp))
        # print(cursor.rowcount, "Record inserted succesfully into Velocity table")
        connection.commit()
    if axis == "y":
        postgres_insert = """ INSERT INTO VELOCITY_Y(sensor_id, velocity, category, time_stamp) VALUES(%s, %s, %s, %s)"""
        cursor.execute(postgres_insert, (ID, v, cat, time_stamp))
        # print(cursor.rowcount, "Record inserted succesfully into Velocity table")
        connection.commit()
    if axis == "z":
        postgres_insert = """ INSERT INTO VELOCITY_Z(sensor_id, velocity, category, time_stamp) VALUES(%s, %s, %s, %s)"""
        cursor.execute(postgres_insert, (ID, v, cat, time_stamp))
        # print(cursor.rowcount, "Record inserted succesfully into Velocity table")
        connection.commit()

# Insert frequency peaks into Postgres DB
def DB_insert_peaks(ID, f_peak, a_peak, time_stamp, axis):
    if axis == "x":
        client.publish(topic_peak_f_x, ('e' + str(f_peak)))
        client.publish(topic_peak_a_x, ('h' + str(a_peak)))
        postgres_insert = """ INSERT INTO FREQ_PEAKS_X(sensor_id, amplitude, freq, time_stamp) VALUES(%s, %s, %s, %s)"""
        cursor.execute(postgres_insert, (ID, a_peak, f_peak, time_stamp))
        # print(cursor.rowcount, "Record inserted succesfully into Velocity table")
        connection.commit()
    if axis == "y":
        client.publish(topic_peak_f_y, ('f' + str(f_peak)))
        client.publish(topic_peak_a_y, ('i' + str(a_peak)))
        postgres_insert = """ INSERT INTO FREQ_PEAKS_Y(sensor_id, amplitude, freq, time_stamp) VALUES(%s, %s, %s, %s)"""
        cursor.execute(postgres_insert, (ID, a_peak, f_peak, time_stamp))
        # print(cursor.rowcount, "Record inserted succesfully into Velocity table")
        connection.commit()
    if axis == "z":
        client.publish(topic_peak_f_z, ('g' + str(f_peak)))
        client.publish(topic_peak_a_z, ('j' + str(a_peak)))
        postgres_insert = """ INSERT INTO FREQ_PEAKS_Z(sensor_id, amplitude, freq, time_stamp) VALUES(%s, %s, %s, %s)"""
        cursor.execute(postgres_insert, (ID, a_peak, f_peak, time_stamp))
        # print(cursor.rowcount, "Record inserted succesfully into Velocity table")
        connection.commit()


# MQTT setup
client = client.Client()

client.message_callback_add("warehouse1/sensor1/x", callback_x)
client.message_callback_add("warehouse1/sensor1/y", callback_y)
client.message_callback_add("warehouse1/sensor1/z", callback_z)

# Username and password to the broker requiered
client.username_pw_set(username="IoTproject", password="Thursday123")
client.on_connect = on_connect
client.connect(hostname)

client.loop_start()


def get_data():
    thread_wait_for_data = threading.Thread(target=wait_for_data)
    thread_wait_for_data.start()
    thread_wait_for_data.join()


while True:

    if __name__ == '__main__':
        time_stamp = datetime.datetime.now()  # - datetime.timedelta(hours=2)

        # print(time_stamp)
        # Wait for all acceleration data to come through via MQTT - WORKING
        get_data()
        DB_insert_sensor_info(sensor_no, location)

        # Calculate FFT
        f_x, a_x = fft_blah(x[-samples:], sample_freq, samples)
        f_y, a_y = fft_blah(y[-samples:], sample_freq, samples)
        f_z, a_z = fft_blah(z[-samples:], sample_freq, samples)

        # Insert FFT data to DB - not used at the moment, it is working though

        # for i in range(half_samples):
        #     insert_FFTx = threading.Thread(target=DB_insert_FFT, args=(sensor_no, f_x[i], a_x[i], time_stamp, "x"))
        #     DB_threads_x.append(insert_FFTx)
        #     insert_FFTy = threading.Thread(target=DB_insert_FFT, args=(sensor_no, f_y[i], a_y[i], time_stamp, "y"))
        #     DB_threads_y.append(insert_FFTy)
        #     insert_FFTz = threading.Thread(target=DB_insert_FFT, args=(sensor_no, f_z[i], a_z[i], time_stamp, "z"))
        #     DB_threads_z.append(insert_FFTz)
        #
        #     insert_FFTx.start()
        #     insert_FFTy.start()
        #     insert_FFTz.start()
        #     insert_FFTx.join()
        #     insert_FFTy.join()
        #     insert_FFTz.join()

        # Calculate velocity
        veloc_x = velocity(f_x, a_x, half_samples)
        veloc_y = velocity(f_y, a_y, half_samples)
        veloc_z = velocity(f_z, a_z, half_samples)

        # Publish velocity data to MQTT
        client.publish(topic_vel_x, ('x' + str(veloc_x)))
        client.publish(topic_vel_y, ('y' + str(veloc_y)))
        client.publish(topic_vel_z, ('z' + str(veloc_z)))

        # Return category of velocity
        vel_cat_x = category(veloc_x)
        vel_cat_y = category(veloc_y)
        vel_cat_z = category(veloc_z)

        # Publish velocity category data to MQTT
        client.publish(topic_cat_x, 'b' + vel_cat_x)
        client.publish(topic_cat_y, 'c' + vel_cat_y)
        client.publish(topic_cat_z, 'd' + vel_cat_z)

        # Insert velocity and category data to DB
        DB_insert_velocity(sensor_no, veloc_x, vel_cat_x, time_stamp, "x")
        DB_insert_velocity(sensor_no, veloc_y, vel_cat_y, time_stamp, "y")
        DB_insert_velocity(sensor_no, veloc_z, vel_cat_z, time_stamp, "z")

        # Find 5 highest amplitudes
        freque_x, amplit_x = find_peaks(f_x, a_x)
        freque_y, amplit_y = find_peaks(f_y, a_y)
        freque_z, amplit_z = find_peaks(f_z, a_z)

        # Insert 5 peaks of each FFT axis to DB
        for k in range(1, 3):
            insert_peaks_x = threading.Thread(target=DB_insert_peaks,
                                              args=(sensor_no, freque_x[k], amplit_x[k], time_stamp, "x"))
            DB_peaks_x.append(insert_peaks_x)
            insert_peaks_y = threading.Thread(target=DB_insert_peaks,
                                              args=(sensor_no, freque_y[k], amplit_y[k], time_stamp, "y"))
            DB_peaks_x.append(insert_peaks_y)
            insert_peaks_z = threading.Thread(target=DB_insert_peaks,
                                              args=(sensor_no, freque_z[k], amplit_z[k], time_stamp, "z"))
            DB_peaks_x.append(insert_peaks_z)

            insert_peaks_x.start()
            insert_peaks_y.start()
            insert_peaks_z.start()
            insert_peaks_x.join()
            insert_peaks_y.join()
            insert_peaks_z.join()

        print("Velocity x: ", veloc_x)
        print("Velocity y: ", veloc_y)
        print("Velocity z: ", veloc_z)
        for i in range(3):
            print("Freq_x = ", freque_x[i], "Amp_x = ", amplit_x[i])
            print("Freq_y = ", freque_y[i], "Amp_y = ", amplit_y[i])
            print("Freq_z = ", freque_z[i], "Amp_z = ", amplit_z[i])

        x = []
        y = []
        z = []
        DB_threads_x = []
        DB_threads_y = []
        DB_threads_z = []
        DB_peaks_x = []
        DB_peaks_y = []
        DB_peaks_z = []

    client.disconnect()
    client.loop_stop()
