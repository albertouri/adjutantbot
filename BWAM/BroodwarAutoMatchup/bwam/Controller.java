package bwam;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.List;

public class Controller {
	public static String CMD_JOIN_GAME = "JOIN";
	public static String CMD_READY = "READY";
	public static String CMD_SWITCH = "SWITCH";
	public static String CMD_EXIT = "EXIT";
	public static String CMD_CLICK_OK = "CLICKOK";
	public static String CMD_DELIM = ":";
	
	private static String CHAOS_LAUNCHER_DIR = "C:\\Program Files\\Chaoslauncher";
	private static String CHAOS_LAUNCHER_EXE = "C:\\Program Files\\Chaoslauncher\\Chaoslauncher.exe";
	private static String BWAPI_INI = "C:\\Program Files\\Starcraft\\bwapi-data\\bwapi.ini";
	private static String BWAPI_INI_NAME = "bwapi.ini";
	private static String AI_DLL_DIRECTORY = "C:\\Program Files\\Starcraft\\bwapi-data\\AI";
	private static String CHAOS_LAUNCHER_PROCESS_NAME = "Chaoslauncher.exe";
	private static String STARCRAFT_PROCESS_NAME = "StarCraft.exe";
	private static String AHKEXE_START_STARCRAFT = "StartChaoslauncher.exe";
	private static String AHKEXE_START_GAME = "StartGame.exe";
	private static boolean isInitialized = false;
	
	public static void init()
	{
		if (! new File(CHAOS_LAUNCHER_EXE).exists())
		{
			CHAOS_LAUNCHER_EXE = CHAOS_LAUNCHER_EXE.replace(
				"C:\\Program Files", "C:\\Program Files (x86)");
			if (! new File(CHAOS_LAUNCHER_EXE).exists())
			{
				System.out.println("Unable to find " + CHAOS_LAUNCHER_EXE);
				System.exit(0);
			}
		}
		
		if (! new File(BWAPI_INI).exists())
		{
			BWAPI_INI = BWAPI_INI.replace(
				"C:\\Program Files", "C:\\Program Files (x86)");
			if (! new File(BWAPI_INI).exists())
			{
				System.out.println("Unable to find " + BWAPI_INI);
				System.exit(0);
			}
		}
		
		if (! new File(AI_DLL_DIRECTORY).exists())
		{
			AI_DLL_DIRECTORY = AI_DLL_DIRECTORY.replace(
				"C:\\Program Files", "C:\\Program Files (x86)");
			if (! new File(AI_DLL_DIRECTORY).exists())
			{
				System.out.println("Unable to find " + AI_DLL_DIRECTORY);
				System.exit(0);
			}
		}
		isInitialized = true;
	}
	
	public static boolean startStarCraft()
	{
		if (!isInitialized) {init();}
		boolean isSuccess = true;
		
		try 
		{
			//Start Chaoslauncher
			Runtime.getRuntime().exec('"' + CHAOS_LAUNCHER_EXE + '"', null, new File(CHAOS_LAUNCHER_DIR));
			
			//Wait for Chaoslauncher to appear
			Thread.sleep(2000);
			
			//Execute AutoHotKey script to start StarCraft
			Runtime.getRuntime().exec('"' + AHKEXE_START_STARCRAFT + '"');
		} 
		catch (Exception e) 
		{
			e.printStackTrace();
			isSuccess = false;
		}
		
		return isSuccess;
	}
	
	public static boolean startGame()
	{
		if (!isInitialized) {init();}
		boolean isSuccess = true;
		
		try 
		{
			//Execute AutoHotKey script to start click OK button and start game
			Runtime.getRuntime().exec('"' + AHKEXE_START_GAME + '"');
		} 
		catch (IOException e) 
		{
			e.printStackTrace();
			isSuccess = false;
		}
		
		return isSuccess;
	}
	
	public static boolean closeStarCraft()
	{
		if (!isInitialized) {init();}
		boolean isSuccess = true;
		
		try 
		{
			//Kill StarCraft process
			Runtime.getRuntime().exec("taskkill /IM " + '"' + STARCRAFT_PROCESS_NAME + '"');
			Thread.sleep(10000);
			
			//Kill ChaosLauncher process
			Runtime.getRuntime().exec("taskkill /IM " + '"' + CHAOS_LAUNCHER_PROCESS_NAME + '"');
			Thread.sleep(20000);
		} 
		catch (Exception e) 
		{
			e.printStackTrace();
			isSuccess = false;
		}
		
		return isSuccess;
	}
	
	public static boolean switchAIModule(String dll, String race, boolean isClient)
	{
		if (!isInitialized) {init();}
		boolean isSuccess = true;
		
		File template = new File(BWAPI_INI_NAME);
	
		try
		{
			//Use template INI text and change dll and race
			List<String> lineList = readFileHelper(template.getAbsolutePath());
			List<String> outList = new ArrayList<String>();
			
			for (String line : lineList)
			{
				if (line.startsWith("ai     = bwapi-data"))
				{
					outList.add("ai     = bwapi-data\\AI\\" + dll);
				}
				else if (line.startsWith("race = "))
				{
					outList.add("race = " + race);
				}
				else if (isClient && line.startsWith("map ="))
				{
					outList.add("map = ");
				}
				else if (isClient && line.startsWith("auto_restart ="))
				{
					outList.add("auto_restart = ON");
				}
				else if (isClient && line.startsWith("tournament ="))
				{
					outList.add("tournament = ");
				}
				else
				{
					outList.add(line);
				}
			}
			
			writeFileHelper(BWAPI_INI, outList);
		}
		catch (Exception e)
		{
			e.printStackTrace();
			isSuccess = false;
		}
		
		return isSuccess;
	}
	
	public static List<String> readFileHelper(String filename) throws IOException 
	{
		ArrayList<String> fileContents = new ArrayList<String>();
		InputStream inStream = null;
		InputStreamReader fileIn = null;
		BufferedReader dataInputStream = null;
		
		try 
		{
			inStream = new FileInputStream(filename);
			fileIn = new InputStreamReader(inStream);
			dataInputStream = new BufferedReader(fileIn);
			
			while (dataInputStream.ready())
			{
				fileContents.add(dataInputStream.readLine());
			}
		} 
		finally 
		{
			if (dataInputStream != null) { dataInputStream.close(); }
			if (fileIn != null) { fileIn.close(); }
			if (inStream != null) {inStream.close(); }
		}
		
		return fileContents;
	}
	
	// Write File Helper
	public static void writeFileHelper(String filename, List<String> textList) throws IOException 
	{
		OutputStream outStream = null;
		OutputStreamWriter outWriter = null;
		BufferedWriter bufferedWriter = null;
		
		try 
		{
			outStream = new FileOutputStream(filename);
			outWriter = new OutputStreamWriter(outStream);
			bufferedWriter = new BufferedWriter(outWriter);
			
			for (String textToWrite : textList)
			{
				bufferedWriter.write(textToWrite + "\r\n");
			}
		} 
		finally 
		{
			if (bufferedWriter != null) { bufferedWriter.close(); }
			if (outWriter != null) { outWriter.close(); }
			if (outStream != null) {outStream.close(); }
		}
	}
}
