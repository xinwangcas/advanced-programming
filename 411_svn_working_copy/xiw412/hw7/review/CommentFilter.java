/*************************************************************************************
NOTE: ORIGINAL PROGRAM WRITTEN BY Quinn Taylor
ORIGINAL DOCUMENTATION: http://cocoaheads.byu.edu/wiki/stripping-comments-source-code
This program has been translated to Java from C++/Python
--------------------------------------------------------------------------------
Translated by: William West
CSC411 - Programming Methods
Homework #7
November 18, 2012
Program Description: Given a source code file, strips comments as specified.
					 For file output, re-direct standard output.
					 
Input Parameters: --input [filename]	Source code to be parsed
				  -L					Strip single-line comments
				  -C					Strip C-Style comments
				  -J					Strip Javadoc comments
				  -H					Strip HeaderDoc comments
*************************************************************************************/


/*********************************LIBRARY CITATION*************************************
*    Title: Apache Commons CLI Library
*    Author: The Apache Software Foundation
*    Date: November 19, 2012
*    Code version: 1.2
*    Availability: http://commons.apache.org/cli/index.html
***************************************************************************************/
import org.apache.commons.cli.*;

import java.io.*;

//9 different possible states for parser
enum State {
	SOURCE, STRING_LITERAL, CHAR_LITERAL, SLASH, SLASH_STAR, COMMENT_LINE, 
	COMMENT_CSTYLE, COMMENT_JAVADOC, COMMENT_HEADERDOC
}

class CommentFilter{
	public static void main(String args[]){
		CommandLineParser parser = new PosixParser();
		String filename = "";
		
		//declaring command line parameters
		Options options = new Options();
		options.addOption("L", "line", false, "strip single-line comments //...\\\\\n");
		options.addOption("C", "cstyle", false, "strip C-style comments		/*...*/");
		options.addOption("J", "javadoc", false, "strip Javadoc comments		/**...*/");
		options.addOption("H", "headerdoc", false, "strip HeaderDoc comments /*!...*/");
		options.addOption(OptionBuilder.withLongOpt("input")
										.withDescription("file from which to read input")
										.hasArg()
										.isRequired()
										.create() );
		try{
			CommandLine line = parser.parse(options, args);
			
			//Check if at least one type of comment style is set
			if(!line.hasOption("L") && !line.hasOption("C") && !line.hasOption("J") && !line.hasOption("H")){
				System.out.println("ERROR: Please specify at least one comment style to strip.");
				System.exit(0);
			}
			
			//Check if filename is valid
			filename = line.getOptionValue("input");
			File file = new File(filename);
			if(!file.exists()){
				System.out.println("ERROR: Specified input file does not exist!");
				System.exit(0);
			}
	
		
			try{
				FileInputStream fis = new FileInputStream(file);
				State state = State.SOURCE;
				char prevChar = ' ';
				char thisChar = ' ';
				
				//********PARSER**********//
				while(fis.available()>0){
					prevChar = thisChar;
					thisChar = (char)fis.read();
					
				   if(state == State.SOURCE){
						if(thisChar == '/'){
							state = State.SLASH;
						}
						else{
							if(thisChar == '"')
								state = State.STRING_LITERAL;
							else if(thisChar == '\'')
								state = State.CHAR_LITERAL;
							System.out.print(thisChar);
						}
					}
						
					else if(state == State.STRING_LITERAL){
						if(thisChar == '"' && prevChar != '\\')
							state = State.SOURCE;
						System.out.print(thisChar);
					}
				 
					else if(state == State.CHAR_LITERAL){
						if(thisChar == '\'' && prevChar != '\\')
							state = State.SOURCE;
						System.out.println(thisChar);
					}
					
					else if(state == State.SLASH){
						if(thisChar == '*')
							state = State.SLASH_STAR;
						else if(thisChar == '/'){
							if(!line.hasOption("L"))
								System.out.print("//");
							state = State.COMMENT_LINE;
						}
						else{
							System.out.print("/");
							System.out.print(thisChar);
							state = State.SOURCE;
						}
					}
				 
					else if(state == State.SLASH_STAR){
						if(thisChar == '*'){
							if(!line.hasOption("J"))
								System.out.print("/**");
							state = State.COMMENT_JAVADOC;
						}
						else if(thisChar == '!'){
							if(!line.hasOption("H"))
								System.out.print("/*!");
							state = State.COMMENT_HEADERDOC;
						}
						else{
							if(!line.hasOption("C")){
								System.out.print("/*");
								System.out.print(thisChar);
							}
							state = State.COMMENT_CSTYLE;
							thisChar = 0;
						}
					}
				 
					else if(state == State.COMMENT_LINE){
						if(thisChar == '\n'){
							System.out.print("\n");
							state = State.SOURCE;
						}
						if(!line.hasOption("L"))
							System.out.print(thisChar);
					}
				 
					else if(state == State.COMMENT_CSTYLE){
						if(!line.hasOption("C"))
							System.out.print(thisChar);
						if(prevChar == '*' && thisChar == '/')
							state = State.SOURCE;
					}
				 
					else if(state == State.COMMENT_JAVADOC){
						if(!line.hasOption("J"))
							System.out.print(thisChar);
						if(prevChar == '*' && thisChar == '/')
							state = State.SOURCE;
					}
				 
					else if(state == State.COMMENT_HEADERDOC){
						if(!line.hasOption("H"))
							System.out.print(thisChar);
						if(prevChar == '*' && thisChar == '/')
							state = State.SOURCE;
					}
				}
				fis.close();
			}
			catch(IOException e){
				System.out.println("Unexpected exception "+e.getMessage());
			}
		}	
		catch(ParseException exp){
			System.out.println("Unexpected exception: "+ exp.getMessage());
		}
	}
}