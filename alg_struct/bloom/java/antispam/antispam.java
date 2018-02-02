package antispam;

import java.io.*;
import java.util.Random;

public class antispam
{
	private static void print(String str)
	{
		System.out.println(str);
	}

	private static void check_correct(bloom filter, String file_name)
	{
		FileInputStream input_stream = null;
		BufferedReader reader = null;
		try {
			input_stream = new FileInputStream(file_name);
			reader = new BufferedReader(new InputStreamReader(input_stream));
			String line = reader.readLine();
			while (line != null && line.length() > 0) {
				if (!filter.find(line)) {
					print("can not find " + line + " in the @bloom filter!");
				}
				line = reader.readLine();
			}
		} catch (Exception ept) {
			ept.printStackTrace();
		} finally {
			try {
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

	private static void check_false(bloom filter)
	{
		int rand_seed = 0X12345678;
		int idx_i = 0;
		int top = 1500000;
		int idx_j = 0;
		int number = 0;
		int rand_str_len = 16;
		int false_num = 0;

		String base = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
		java.util.Random rand = new java.util.Random(rand_seed);
		for (idx_i = 0; idx_i < top; idx_i++) {
			StringBuffer rand_str = new StringBuffer();
			for (idx_j = 0; idx_j < rand_str_len; idx_j++) {
				number = Math.abs(rand.nextInt());
				rand_str.append(base.charAt(number % base.length()));
			}
			//print("idx = " + idx_i + ", string = " + rand_str.toString());
			if (filter.find(rand_str.toString())) {
				false_num++;
				print("index:" + false_num + ", find random string " + rand_str.toString() + " in the @bloom filter!");
			}
		}

		print("false number:" + false_num + ", false positive rate:" + false_num + "/" + top);
	}

	public static void main(String[] args)
	{
		if (args.length != 2) {
			print("Fatal error: you should input a advertisement md5 list file and its bloom dict file.");
			return;
		}
		String ad_md5_file = args[0];
		String ad_dict_file = args[1];

		File input_file = null;
		FileInputStream input_stream = null;
		DataInputStream reader = null;

		bloom filter;
		try {
			input_file = new File(ad_dict_file);
			input_stream = new FileInputStream(input_file);
			int file_length = (int)(input_file.length());
			//print("file length:" + file_length);
			reader = new DataInputStream(input_stream);
			// filter = new bloom();

			// check version
			int length = bloom.BLOOM_VERSION.length();
			byte[] file_version = new byte[length];
			reader.read(file_version, 0, length);
			String file_version_string = new String(file_version);
			//print(file_version_string);
			if (!file_version_string.equals(bloom.BLOOM_VERSION)) {
				print("error: the dictionary file " + ad_dict_file + " which you input is illegal!");
			}

			// rebuild the bloom object
			length = file_length - length;
			byte[] dict_buffer = new byte[length];
			// print("dict length = " + length);
			reader.read(dict_buffer, 0, length);
			filter = new bloom(dict_buffer);

			// check md5 list
			check_correct(filter, ad_md5_file);

			// check random number
			check_false(filter);
		} catch (Exception ept) {
			ept.printStackTrace();
		} finally {
			try {
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

