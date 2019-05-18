/**
 * Decoder for the "custom" data format send by a buoy, configured on "The Things Network".
 * 
 * Developed by Benjamin Van der Smissen
 * Modified by Brecht Van Eeckhoudt
 * 
 * More documentation about these custom packets can be found in the 
 * software documentation in "/lora/lpp.c" for the following methods:
 *  - LPP_AddMeasurements
 *  - LPP_AddStormDetected
 *  - LPP_AddCableBroken
 *  - LPP_AddStatus
 * 
 * Information gathered from:
 *  - https://dramco.be/tutorials/low-power-iot/ieee-sensors-2017/store-sensor-data-in-the-cloud
 *  - https://tago.elevio.help/en/articles/118-building-your-own-parser
 */


 /**
  * Function that does the decoding, calls other functions if necessary.
  * @param {*} bytes The received "raw" bytes.
  * @param {*} port The port the bytes were received on (not used...)
  */
function Decoder (bytes, port) {
	var decoded = {};
	decoded.BatteryVoltage = [];
	decoded.InternalTemperature = [];
	decoded.ExternalTemperature = [];
	decoded.StormDetected = [];
	decoded.CableBroken = [];
	decoded.Status = [];

	var count = 0; 
	var NR_of_Meas = bytes[0];
	var type;
	count++;
 
	while (count < bytes.length) {
		switch (bytes[count]) {

			// 0x10 = Battery voltage channel 
			case 0x10:
				count++;
				if (bytes[count] === 0x02) { // 0x02 = Analog Input (Cayenne LPP datatype)
					count++;
					type = 0x02;
					decoded.BatteryVoltage = bytesToArray2(bytes, NR_of_Meas, count, type);
					count += NR_of_Meas*2;
				}
				break;

			// 0x11 = Internal temperature channel
			case 0x11:
				count++;
				if (bytes[count] === 0x67) { // 0x67 = Temperature (0.1 °C Signed MSB - Cayenne LPP datatype)
					count++;
					type = 0x67;
					decoded.InternalTemperature = bytesToArray2(bytes, NR_of_Meas, count, type);
					count += NR_of_Meas*2 ;
				}
				break;

			// 0x12 = External temperature channel
			case 0x12:
				count++;
				if (bytes[count] === 0x67) { // 0x67 = Temperature (0.1 °C Signed MSB - Cayenne LPP datatype)
					count++;
					type = 0x67;
					decoded.ExternalTemperature = bytesToArray2(bytes, NR_of_Meas, count, type);
					count += NR_of_Meas*2;
				}
				break;

			// 0x13 = Storm detected channel
			case 0x13:
				count++;
				if (bytes[count] === 0x00) { // 0x00 = Digital input (Cayenne LPP datatype)
					count++;
					decoded.StormDetected = bytesToArray1(bytes, NR_of_Meas, count);
					count += NR_of_Meas;
				}
				break;

			// 0x14 = Cable broken channel
			case 0x14:
				count++;
				if (bytes[count] === 0x00) { // 0x00 = Digital input (Cayenne LPP datatype)
					count++;
					decoded.CableBroken = bytesToArray1(bytes, NR_of_Meas, count);
					count += NR_of_Meas;
				}
				break;

			// 0x15 = Status channel
			case 0x15:
				count++;
				if (bytes[count] === 0x00) { // 0x00 = Digital input (Cayenne LPP datatype)
					count++;
					decoded.Status = bytesToArray1(bytes, NR_of_Meas, count);
					count += NR_of_Meas;
				}
				break;
		}
	}

  return decoded;
}


/**
 * Function to convert bytes to an array when a specific Cayenne LPP datatype is given.
 * @param {*} bytes The "raw" bytes to convert.
 * @param {*} NR_of_Meas The number of measurements in the raw bytes.
 * @param {*} count Counter value.
 * @param {*} type The Cayenne LPP datatype.
 */
function bytesToArray2(bytes, NR_of_Meas, count, type) {
	var data_size = 2;
	var num_byte_el = bytes.length;
	var readings = [];

	for (var reading_id = 0; reading_id < NR_of_Meas; reading_id++) {
		var value = ( ( (bytes[reading_id+count] << 8) | bytes[reading_id+count+1]) << 16) >> 16;

		// Send float values are multiplied by 1000 (24.6 = 24600)
		if (type === 0x67) readings.push(parseFloat(value)/10.0);
		else readings.push(parseFloat(value)/100.0);
		
		count += 1;
	}

	console.log(readings);
	return readings;
}


/**
 * Function to convert bytes to an array.
 * @param {*} bytes The "raw" bytes to convert.
 * @param {*} NR_of_Meas The number of measurements in the raw bytes.
 * @param {*} count Counter value.
 */
function bytesToArray1(bytes, NR_of_Meas, count) {
	var data_size = 1;
	var num_byte_el = bytes.length;
	var readings = [];

	for (var reading_id = 0; reading_id < NR_of_Meas; reading_id++) {
		var value = bytes[reading_id+count];

		readings.push(parseInt(value));
		
		count += 0;
	}

	console.log(readings);
	return readings;
}
