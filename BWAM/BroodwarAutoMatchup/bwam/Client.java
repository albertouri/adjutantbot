package bwam;
// File Name Client.java

import java.net.*;
import java.io.*;

public class Client extends Thread
{
	private String serverName;
	private int port;
	private boolean inGame;
	
	public Client(int port, String serverName) throws IOException
	{
		this.inGame = false;
		this.serverName = serverName;
		this.port = port;
	}
	
	public void run()
	{
		Socket clientSocket = null;
		DataInputStream in = null;
		DataOutputStream out = null;
		boolean stillLoop = true;
		
		//Establish connection
		try
		{
			System.out.println("Attempting to connect to " 
					+ this.serverName + " on " + this.port);
			clientSocket = new Socket(this.serverName, this.port);
			out = new DataOutputStream(clientSocket.getOutputStream());
			in = new DataInputStream(clientSocket.getInputStream());
			System.out.println("Connected succesfully");
		} catch (IOException e) {
			e.printStackTrace();
			stillLoop = false;
		}
		
		while(stillLoop)
		{
			//Receive Messages
			try
			{		
				//Wait for commands from host
				String command = in.readUTF();
				
				String[] commandParts = command.split(Controller.CMD_DELIM);
				String baseCommand = commandParts[0];
				
				if (baseCommand.equals(Controller.CMD_SWITCH))
				{
					System.out.println("Switching");
					if (commandParts.length == 3)
					{
						String clientDll = commandParts[1];
						String clientRace = commandParts[2];
						
						if (this.inGame)
						{
							Controller.closeStarCraft();
							this.inGame = false;
						}
						
						Controller.switchAIModule(clientDll, clientRace, true);
						
						//Let host know switch is complete
						out.writeUTF(Controller.CMD_READY);
					}
					else
					{
						System.out.println("Malformed switch command from host: " + command);
						stillLoop = false;
					}
				}
				else if (baseCommand.equals(Controller.CMD_JOIN_GAME))
				{
					System.out.println("Joining game");
					Controller.startStarCraft();
					this.inGame = true;
					
					//Stall to make sure we have joined
					Thread.sleep(15000);
				}
				else if (baseCommand.equals(Controller.CMD_EXIT))
				{
					System.out.println("Exiting");
					Controller.closeStarCraft();
					this.inGame = false;
					stillLoop = false;
				}
				else
				{
					System.out.println("Unexpected command: " + command);
					stillLoop = false;
				}
			}
			catch(Exception e)
			{
				e.printStackTrace();
				stillLoop = false;
			}
		}
		
		if (clientSocket != null)
		{
			try {clientSocket.close();} catch (IOException e) {e.printStackTrace();}
		}
	}
	
	public static void main(String [] args)
	{
		int port = Integer.parseInt(args[0]);
		String serverName = args[1];
		
		try
		{
			Thread t = new Client(port, serverName);
			t.start();
		}
		catch(IOException e)
		{
			e.printStackTrace();
		}
	}
}