package antispam;

import java.io.*;

public class converter
{
	private static void print(String str)
	{
		System.out.println(str);
	}

	public static void main(String []args)
	{
		if (args.length != 2) {
			print("Fatal error: you should input a advertisement md5 list file and its bloom dict file name.");
			return;
		}
		String ad_md5_file = args[0];
		String ad_dict_file = args[1];

		//bloom
		FileInputStream input_stream = null;
		BufferedReader reader = null;
		FileOutputStream output_stream = null;
		DataOutputStream writer = null;
		bloom filter = null;
		try {
			input_stream = new FileInputStream(ad_md5_file);
			reader = new BufferedReader(new InputStreamReader(input_stream)); //(new FileReader(ad_md5_file));
			String line = reader.readLine();
			// get line number
			int line_num = 0;
			while (line != null && line.length() > 0) {
				//print(line);
				line_num++;
				line = reader.readLine();
			}

			// initialise a bloom filter object
			filter = new bloom(line_num);

			// reset to the beginning of the file
			// and intput every line string into the bloom filter
			input_stream.getChannel().position(0);
			line = reader.readLine();
			while (line != null && line.length() > 0) {
				filter.put(line);
				line = reader.readLine();
			}
			byte[] dict_buffer = filter.data();

			// write the data of @filter to a file
			output_stream = new FileOutputStream(ad_dict_file);
			writer = new DataOutputStream(output_stream);
			writer.writeBytes(filter.BLOOM_VERSION);
			writer.write(dict_buffer, 0, dict_buffer.length);
		} catch (Exception ept) {
			ept.printStackTrace();
		} finally {
			try {
				if (writer != null) {
					writer.close();
				}

				if (output_stream != null) {
					output_stream.close();
				}

				if (reader != null) {
					reader.close();
				}

				if (input_stream != null) {
					input_stream.close();
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
}

