# Linux Intro: Your First HPC Terminal Demo

Welcome to this short Linux activity.

In this activity, you will use the Linux terminal to explore the computer, run commands, make the CPU do work, and automate a small task.

You do not need to memorise every command. The goal is to try things, see what happens, and understand why Linux is useful in high performance computing, science, engineering, AI, cybersecurity, and research.

> Tip: Take your time. If something does not work, check the spelling, spaces, and capital letters in the command.

## Table of contents

- [What you will learn](#what-you-will-learn)
- [1. Open a terminal](#1-open-a-terminal)
- [2. Find out where you are](#2-find-out-where-you-are)
- [3. Meet your computer](#3-meet-your-computer)
- [4. Watch the computer work](#4-watch-the-computer-work)
- [5. Make the CPU do work](#5-make-the-cpu-do-work)
- [6. Create many files automatically](#6-create-many-files-automatically)
- [7. Search through all the files](#7-search-through-all-the-files)
- [8. Run a tiny Python command](#8-run-a-tiny-python-command)
- [9. How this connects to HPC](#9-how-this-connects-to-hpc)
- [Useful command summary](#useful-command-summary)
- [Challenge section](#challenge-section)
- [Clean up](#clean-up)

## What you will learn

By the end of this activity, you will have used Linux to:

- Find your current folder
- List files and folders
- Check the computer name
- Check CPU, memory, and storage information
- Watch programs running in real time
- Make the computer do work
- Run several tasks at the same time
- Create and search many files automatically
- Run a small Python command from the terminal

> Note: Linux commands are case-sensitive. `Desktop`, `desktop`, and `DESKTOP` are different names.

## 1. Open a terminal

On most Linux computers, you can open a terminal by pressing:

`Ctrl + Alt + T`

You can also search for `Terminal` in the application menu.

The terminal lets you control the computer by typing commands.

## 2. Find out where you are

Type this command and press Enter:

```bash
pwd
```

This shows your current folder.

Now type:

```bash
ls
```

This lists the files and folders in your current folder.

### What does `pwd` mean?

`pwd` stands for `print working directory`.

It tells you where you are in the file system.

## 3. Meet your computer

In this section, you will ask the computer for information about itself.

### Show the computer name

```bash
hostname
```

On an HPC cluster, this command tells you which compute node you are using.

### Show the operating system

```bash
cat /etc/os-release
```

This tells you which Linux distribution is installed.

### Show CPU information

```bash
lscpu
```

This shows information about the processor.

Look for:

- CPU model
- Number of CPUs
- Number of cores
- Threads per core

You can also use this shorter version:

```bash
lscpu | grep -E "Model name|CPU\(s\)|Core|Thread"
```

### Show memory usage

```bash
free -h
```

This shows how much RAM the computer has and how much is being used.

### Show storage space

```bash
df -h
```

This shows how much disk space is available.

## 4. Watch the computer work

Run:

```bash
top
```

This shows live information about what the computer is doing.

Look for:

- CPU usage
- Memory usage
- Running programs

Press:

`q`

to quit `top`.

If `htop` is installed, you can also try:

```bash
htop
```

This gives a more visual version of `top`.

## 5. Make the CPU do work

Run this command:

```bash
timeout 5 yes > /dev/null
```

This makes the CPU do work for 5 seconds.

What is happening?

| Part          | Meaning                           |
| ------------- | --------------------------------- |
| `yes`         | Prints text repeatedly            |
| `> /dev/null` | Throws the output away            |
| `timeout 5`   | Stops the command after 5 seconds |

Now try running several workers at once:

```bash
for i in {1..4}; do timeout 10 yes > /dev/null & done
```

Then quickly run:

```bash
top
```

You should see the CPU working harder.

> Important: This is a small example of parallel computing. Instead of doing one task, the computer is doing several tasks at the same time.

## 6. Create many files automatically

Create a new folder:

```bash
mkdir my_hpc_demo
```

Move into it:

```bash
cd my_hpc_demo
```

Create 50 files automatically:

```bash
for i in {1..50}; do echo "This is file number $i" > file_$i.txt; done
```

List the files:

```bash
ls
```

Count the files:

```bash
ls | wc -l
```

The `|` symbol is called a pipe. It sends the output of one command into another command.

In this example:

- `ls` lists the files
- `wc -l` counts the number of lines
- Together, they count the number of files shown by `ls`

## 7. Search through all the files

Search for file number 42:

```bash
grep "number 42" *.txt
```

This searches inside all text files for the words `number 42`.

Try searching for another number:

```bash
grep "number 17" *.txt
```

Linux can search many files at once, which is useful when working with large amounts of data.

## 8. Run a tiny Python command

Run:

```bash
python3 -c 'for i in range(10): print("HPC is " + "*"*i)'
```

This runs a small Python program directly from the terminal.

Linux is often used with programming languages like:

- Python
- C
- C++
- Fortran
- Bash
- R

## 9. How this connects to HPC

High performance computing is about using powerful computers to solve large problems.

Linux is useful because it lets you:

- Control computers directly
- Automate repeated work
- Run programs from the command line
- Work with many files quickly
- Monitor CPU, memory, and storage
- Run many tasks at once
- Use remote servers and clusters

Most HPC systems run Linux, so learning the terminal is one of the best ways to get started.

## Useful command summary

| Command               | What it does                   |
| --------------------- | ------------------------------ |
| `pwd`                 | Shows your current folder      |
| `ls`                  | Lists files and folders        |
| `cd folder_name`      | Moves into a folder            |
| `mkdir folder_name`   | Creates a folder               |
| `hostname`            | Shows the computer name        |
| `cat /etc/os-release` | Shows the Linux version        |
| `lscpu`               | Shows CPU information          |
| `free -h`             | Shows memory usage             |
| `df -h`               | Shows storage usage            |
| `top`                 | Shows running programs         |
| `q`                   | Quits `top`                    |
| `grep "text" files`   | Searches for text inside files |
| `wc -l`               | Counts lines                   |
| `python3`             | Runs Python                    |

## Challenge section

Try these after the main demo.

### Challenge 1: Create 100 files

```bash
for i in {1..100}; do echo "Linux made file $i" > challenge_$i.txt; done
```

Check how many files were created:

```bash
ls challenge_*.txt | wc -l
```

### Challenge 2: Search for your favourite number

Replace `25` with your own number:

```bash
grep "25" challenge_*.txt
```

### Challenge 3: Make the computer work for 3 seconds

```bash
timeout 3 yes > /dev/null
```

## Clean up

When you are done, move out of the folder:

```bash
cd ..
```

Remove the demo folder:

```bash
rm -r my_hpc_demo
```

> Warning: Be careful with `rm -r`. It deletes folders and their contents.

## Sources and further reading

These resources can help you continue learning Linux after the workshop.

| Resource                                                                                                                                        | What it is useful for                                                                                    |
| ----------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------- |
| [TryHackMe: Linux Fundamentals](https://tryhackme.com/module/linux-fundamentals)                                                                | A beginner-friendly interactive Linux learning module. Useful if you want guided practice in a browser.  |
| [GeeksforGeeks: Linux Tutorial](https://www.geeksforgeeks.org/linux-unix/linux-tutorial/)                                                       | A written Linux tutorial with explanations of common commands and concepts. Useful as a quick reference. |
| [Royal Holloway: Introduction to Linux PDF](https://intranet.royalholloway.ac.uk/computerscience/computerdocumentation/introductiontolinux.pdf) | A more traditional introductory Linux guide. Useful if you want a structured document to read through.   |

> Note: You do not need to complete all of these. Pick one resource and practise a few commands at a time.

## Final idea

You do not need a supercomputer to start learning HPC.

You can start with a terminal, a few commands, and curiosity.
