# MiniCompilerForSimpleLanguage
Mini Compiler for Simple Language 
Project Documentation 
COMP343 — Compiler Construction 
Group Members 
1. Maryam Ameen 
2. Mahnoor Ali 
3. Amina Fiaz 
4. Ariba Fatima 
5. Aqeela Tahir


<b>1. What is a Mini Compiler? </b>
A compiler is a program that reads code written by a human (called source code) and translates it 
into a form the computer can actually run. Think of it like a translator between two languages: 
the language we write in and the language the machine understands. 
Our Mini Compiler does this for a simplified programming language. It takes our code step by 
step through several stages, catches any mistakes (like grammar errors or using a variable before 
defining it), and finally produces output or executable instructions. 
Even though it is called “mini”, it covers all the core stages a real-world compiler goes through 
which will make it a complete learning project. 

<b>How does a Compiler Work in Simple Terms? </b>
Imagine writing a sentence in English and giving it to a translator who: 
• Reads each word one by one (Lexical Analysis) 
• Checks if the sentence follows the rules of grammar (Syntax Analysis) 
• Makes sure the meaning is valid (Semantic Analysis) 
• Rewrites it in a simpler form for translation (IR Generation) 
• Cuts out unnecessary words (Optimization) 
• Delivers the final translation (Code Generation) 
Each of these steps is a “phase” of the compiler. If something goes wrong at any phase, an error 
is reported and the process stops. 

<b>2. Workflow Diagram</b> 
The workflow diagram below shows the overall path our source code takes from the moment we 
write it to the moment we see a result. Each box is a stage, and the arrows show what happens 
next. Error paths branch off when something goes wrong.


