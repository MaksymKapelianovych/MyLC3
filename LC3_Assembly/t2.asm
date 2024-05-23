	.ORIG x3000
	AND R0,R0,#0
Label	
	ADD R0,R0,#75 ;sdfdf
	;sdf 
	;
	OUT
	TRAP x25
    .FILL #34
	.FILL 75
	JSR Label
	.END