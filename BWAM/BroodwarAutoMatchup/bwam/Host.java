package bwam;
// File Name Host.java

import java.net.*;
import java.util.ArrayList;
import java.util.List;
import java.io.*;

public class Host extends Thread
{
	/**
	 * This file is used to determine when a game of StarCraft has ended. 
	 * We expect the Tournament module to create this file at the end of a game.  
	 */
	private String GAME_DONE_FLAG_FILE = "C:\\BroodwarAutoMatchup_GameOverFlag.txt";
	
	private List<TestSeries> testSeriesList;
	private ServerSocket serverSocket;
	private boolean inGame; 
	
	public Host(int port, String cfgFilePath) throws IOException
	{
		inGame = false;
		serverSocket = new ServerSocket(port);
		serverSocket.setSoTimeout(10000);
		
		if (new File(cfgFilePath).exists())
		{
			testSeriesList = parseCfgFile(cfgFilePath);
		}
		else
		{
			System.out.println("Could not find " + cfgFilePath);
		}
	}

	public void run()
	{
		DataInputStream in = null;
		DataOutputStream out = null;
		Socket server = null;
		TestSeries currentSeries = testSeriesList.get(0);
		int seriesIndex = 0;
		int gameCount = 0;
		boolean stillLoop = true;
		
		//Establish connection
		System.out.println("Waiting 10 seconds for client on port " 
			+ serverSocket.getLocalPort() + "...");
		try {
			server = serverSocket.accept();
			System.out.println("Just connected to "
					+ server.getRemoteSocketAddress());
			out = new DataOutputStream(server.getOutputStream());
			in = new DataInputStream(server.getInputStream());
		} catch (IOException e) {
			e.printStackTrace();
			stillLoop = false;
		}
		
		while(stillLoop)
		{
			//Receive Messages
			try
			{				
				//Create a new game
				if (! inGame)
				{
					//Switch to correct host AI
					System.out.println("Switching AI module/race");
					Controller.switchAIModule(currentSeries.hostAIDll, currentSeries.hostAIRace, false);
					
					//Create the host game
					System.out.println("Starting StarCraft");
					Controller.startStarCraft();
					inGame = true;
					Thread.sleep(5000); //let game come up
					
					//Tell client to switch
					System.out.println("Telling client to switch");
					out.writeUTF(Controller.CMD_SWITCH
						+ Controller.CMD_DELIM + currentSeries.clientAIDll
						+ Controller.CMD_DELIM + currentSeries.clientAIRace);

					//Wait until client is ready
					System.out.println("Waiting for client to switch");
					String response = in.readUTF();

					//Tell client to join the game
					System.out.println("Tell client to join game");
					out.writeUTF(Controller.CMD_JOIN_GAME);
				} 
				else
				{
					//Check if game is done
					File f = new File(GAME_DONE_FLAG_FILE);
					
					if (f.exists())
					{
						//Game complete
						gameCount++;
						
						if (gameCount >= currentSeries.numberOfGames)
						{
							seriesIndex++;
							
							if (seriesIndex >= testSeriesList.size())
							{
								System.out.println("Tests complete");
								stillLoop = false;
								Controller.closeStarCraft();
								
								//Tell client exit everything
								out.writeUTF(Controller.CMD_EXIT);								
							}
							else
							{
								gameCount = 0;
								currentSeries = testSeriesList.get(seriesIndex);
								Controller.closeStarCraft();
								inGame = false;
							}
						}
						else
						{
							try { Thread.sleep(15000); } catch (Exception e) {};
							Controller.startGame();
						}
						
						f.delete();
					}
				}
			}
			catch(Exception e)
			{
				e.printStackTrace();
				stillLoop = false;
			}
		}
		
		if (server != null)
		{
			try {server.close();} catch (IOException e) {e.printStackTrace();}
		}
	}
		
	private List<TestSeries> parseCfgFile(String filename)
	{
		List<TestSeries> testSeriesList = new ArrayList<TestSeries>();
		List<String> fileLines = null;
		
		try {
			fileLines = Controller.readFileHelper(filename);
		} catch (IOException e) {
			e.printStackTrace();
			System.exit(1);
		}

		for (String line : fileLines)
		{
			if (line.startsWith("//") || line.equals("")) {continue;}
			
			String[] lineParts = line.split("\\t+");
			
			if (lineParts.length >= 5)
			{
				String hostAIDll = lineParts[0];
				String hostAIRace = lineParts[1];
				String clientAIDll = lineParts[2];
				String clientAIRace = lineParts[3];
				int numberOfGames = Integer.parseInt(lineParts[4]);
				
				testSeriesList.add(new TestSeries(hostAIDll, hostAIRace, 
					clientAIDll, clientAIRace, numberOfGames));
			}
			else
			{
				System.out.println("Poorly formed configuration line: " + line);
			}
		}
		
		return testSeriesList;
	}

	
	public static void main(String [] args)
	{
		int port = Integer.parseInt(args[0]);
		String cfgFile = args[1];
		
		try
		{
			Thread t = new Host(port, cfgFile);
			t.start();
		}
		catch(IOException e)
		{
			e.printStackTrace();
		}
	}
	
	private class TestSeries
	{
		String hostAIDll;
		String hostAIRace;
		String clientAIDll;
		String clientAIRace;
		int numberOfGames;
		
		public TestSeries(String hd, String hr, String cd, String cr, int n)
		{
			this.hostAIDll = hd;
			this.hostAIRace = hr;
			this.clientAIDll = cd;
			this.clientAIRace = cr;
			this.numberOfGames = n;
		}
	}
}