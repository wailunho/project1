#testing for ;
#clear; ls; echo good			#passed

#testing < and >
#cat < test3 > test2			#passed

#testing |
#ls 	| echo 111			#passed
#echo 666 | cat
#ls | sort					#passed

#testing &&
#true && false && ls 			#passed

echo aaa && echo ccc && dd && echo bbb    #failed
echo hi
echo hello
aaa && echo ggg						

#testing ||
#-1 || -1 || -1 || ls		    #passed
#ccc || echo aaa || dd || echo bbb    #passed
#echo hi
#echo hello

#testing for multiple commands	#passed
#echo hi
#echo hello

#: : : 							#failed

#testing for subshell command
#(echo 11222 > test2	)		#passed

