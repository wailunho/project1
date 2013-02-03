#testing for ;
clear; ls; echo good			#passed

#testing < and >
echo this is a test > test3
cat < test3 > test2			#passed
rm test3
rm test2

#testing |
ls 	| echo 111			#passed
echo 666 | cat > test2   # passed
ls | sort					#passed

#testing &&
true && false && ls 			#passed

echo aaa && echo ccc && 123321 && echo bbb    #passed
echo hi
echo hello
aaa && echo ggg		

#testing ||
-1 || -1 || -1 || ls		    #passed
ccc || echo aaa || dd || echo bbb    #passed
echo hi
echo hello

#testing for multiple commands	#passed
echo hi
echo hello

: : : 							#passed

testing for subshell command
(echo 11222 > test2	)		#passed
rm test2

