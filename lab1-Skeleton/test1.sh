#testing for ;
#clear; ls; echo good			#passed

#testing for ;
#clear; ls; echo good			#passed

#testing < and >
#cat < test3 > test2			#passed

#testing |
#ls | sort						#failed

#true && ls #passed
#false || ls					#failed

#testing for multiple commands	#passed
#echo hi
#echo hello

#: : : 							#failed