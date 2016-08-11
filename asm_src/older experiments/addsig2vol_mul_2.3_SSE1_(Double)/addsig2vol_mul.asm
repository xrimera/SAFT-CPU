[segment .text align=16]
global	_as2v_mul				  ; window
global as2v_mul


;void as2v(double* bild, double* AScan,int n_AScan, double* buffer, double* index_vol, int n_index, double* rec_pos,double* em2pix, double *speed, double* resolution,double* time intervall,double* AScan_complex, double* buffer_complex, double* bild)+68

;;;;;;
;old ;                 new
;;;;;;
;28=*bild
;32=*Ascan
;36=n_Ascan
;40=*buffer
;44=*pixel_index      ->  *pixel_vect[1 1 1]
;48=n_index           ->  n_pixX
;52=*receiverpos
;56=*dist_em_pixel    ->  *senderpos
;60=*speed
;64=*resolut
;68=*time_int
;72=*Ascan_complex
;76=*buff_complex
;80=*bild_compl
;84=                  ->  n_pixY
;88=                  ->  n_pixZ
;92=                  ->  *IMAGE_SUM
;96=                  ->  *IMAGE_SUM_compl
;100=                 ->  *IMAGE_MAP
as2v_mul:					  ; Unix
_as2v_mul:					  ; Windows

	push ebp
	push esi
	push edi
	push ebx	 ;+12 (diff:esp+8)

	push eax	 ;buffer for width  +8 (diff esp+12)
	push eax	 ;buffer for CTW    +4 (diff esp+16)

	fninit		 ;init fpu

     ;calc width fpu
	;FNSTCW [esp+4]   ;buffering in stack (16bit!) + FWAIT to ensure save is complete (FSTCW without wait)
	;mov ax,[esp+4]
	;and ax,0F3FFh    ;delete 2 bits
	;or  ax,0000100000000000b ;round up +inf
	;mov [esp+6],ax   ;new CTRWORD
	;FLDCW [esp+6]

	;fldz                 ;load zero
	;fld1                 ;load 1
	;mov eax,[esp+68]     ;time interval
	    ;fld qword [eax]     ;DOUBLE = qword
	;fld dword [eax]      ;SINGLE = dword
	;mov eax,[esp+64]     ;resolution
	     ;fld qword [eax]       ;DOUBLE = qword
	;fld dword [eax]      ;SINGLE = dword
	;mov eax,[esp+60]     ;speed
		;fld qword [eax]       ;DOUBLE = qword
	;fld dword [eax]      ;SINGLE = dword

	;fdivp st1,st0        ; res/speed; destination/source ,pop
	;fdivrp st1,st0       ; ergeb/timeinterv; source/dest ,pop -> width now in st0

	;fcom st0,st1         ; z.b.: 0.4 cmp 1  -> lower
	;fcmovb st0,st2       ; transfer st2 to st0 if below -> width<1 -> interpol not sum!

	;mov [esp+8],dword 2
	;fidiv dword [esp+8]  ; halbieren
	;fistp dword [esp+8]  ; width to int32 (ceiling round)

	;FLDCW [esp+4]        ;restore old CTRWORD
	;fstp    st0
	;fstp    st0          ; -> fpu stack emtpy now
       ;end calc width fpu
       ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

     ;;;calc width with SSE1
      mov eax,1
      cvtsi2ss xmm0,eax
      mov eax,2
      cvtsi2ss xmm1,eax
      mov eax,[esp+68]	   ;time interval
      movss xmm2,[eax]
      mov eax,[esp+64]	   ;resolution
      movss xmm3,[eax]
      mov eax,[esp+60]	   ;speed
      movss xmm4,[eax]

      divss xmm3,xmm4	   ;res/speed -> time
      divss xmm3,xmm2	   ;time/timeinterv. -> number of samples -> width
      addss xmm3,xmm0	   ;width+1  (against floor)
      divss xmm3,xmm1	   ;width/2
      cvttss2si eax,xmm3   ;to int32 (floor ROUNDING)
     ;;;end  calc width with SSE1

     ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
     ;;;width to big?
	mov ecx,[esp+36]  ;n_Ascan
	shr ecx,1	  ;n_aScan/2
	sub ecx,2
	cmp eax,ecx
	jl xs_int_check   ;width < (n_ascan/2)-2
	mov eax,ecx	  ;set Width to maximum ((n_ascan/2)-2)

      ;xsum or interpol?
      xs_int_check:
	cmp eax,0	  ;width 0 check (and negative values!)
	jg xsum
	jmp interpol
      ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



     ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
     ;;;Xsum path
	mov [esp+8],eax   ;width buffer

     xsum:
	mov ebx,[esp+40]  ;*Buffer
	mov edx,[esp+32]  ;*AScan
	mov edi,eax	  ;width
	mov esi,[esp+36]  ;n_AScan
			  ;eax = width

	fld qword [edx]   ;AScan[0]
      b0:
	add edx,8
	fadd qword [edx]
	dec eax 	  ;ecx = width
	jnz b0		  ;AScan[width]

	mov ecx,edi	  ;Width
	fst qword [ebx]   ;buffer[0]

      b1:
	add edx,8
	fadd qword [edx]
	add ebx,8
	fst qword [ebx]   ;buffer[1]
	dec ecx
	jnz b1

	mov eax,edx	  ; edx ->vorauseilend um width
	mov ecx,edi	  ; width
	shl ecx,4	  ;imul ecx,16       ; 2*width ->addr
	add ecx,8
	sub eax, ecx	  ;hinterher um 2*width +1

	mov ecx,esi	  ;n_AScan
	sub ecx,edi	  ; -width
	sub ecx,edi	  ; -width
	sub ecx,1	  ;n_AScan-2*Width-1

	mov ebp,8
      b2:
	fadd qword [edx+ebp] ;vorauseilend
	fsub qword [eax+ebp] ;hinterher
	fst qword [ebx+ebp]  ;buffer[ebx]
	add ebp,8  ;ebx,8
	sub ecx,1
	jnz b2		     ; n_AScan - width

	add edx,ebp
	add eax,ebp
	add ebx,ebp

	mov ecx,edi   ;width
      b3:
       fsub qword [eax]
	add eax,8	  ;hinterher
       fst qword [ebx]	  ;buffer[ebx]
	add ebx,8

	dec ecx
	jnz b3		  ;n_AScan

	fstp  st0	  ;pop st0
     ;end create xsum buffer_real

     ;l_interpol_real (range 5x)
	mov edi,[esp+40]  ;*Buffer_read[0]    ;;;;;[esp+32] *Ascan[0]
	mov ecx,[esp+36]  ;n_AScan
	dec ecx
	shl ecx,3	  ;double ptr *ascan[2999]
	add edi,ecx	  ;double ptr *ascan[2999]

	mov esi,[esp+40]  ;*Buffer_write[0]
	mov ebp,esi	  ;*Buffer_write[0]
	mov edx,[esp+36]  ;n_AScan
	mov eax,5	  ;interp_range!!!
	mul edx 	  ;eax*edx  result 64bit: [EDX EAX] !!!!
	dec eax
	shl eax,3	  ;double ptr *buffer[14999]
	add esi,eax	  ;double ptr *buffer[14999]

	;prefill FPU Stack
	fld1
	fld1
	fadd st1,st0
	fldz
	fldz
	fldz
	fadd st0,st4  ;st0=2
	fadd st0,st4  ;st0=4
	fadd st0,st0  ;st0=8
	fadd st0,st4  ;st4=2, st3=1; st2=0; st1=0; st0=10

	fdiv st4,st0	   ;0,2 = st4; st3 = ???(buffer), st2 = ???(buffer)  st1 = ???(buffer), st0 = ???(buffer)

	;last 4pixel position
	fld qword [edi]    ;*Ascan[2999]
	fmul st0,st5	   ;0,2 * (*Ascan[2999])
	fst qword [esi]    ; *Buffer[14999]  0.2
	fst st1
	fadd st0,st1
	fst qword [esi-8]  ; *Buffer[14998]  0.4
	fadd st0,st1
	fst qword [esi-16] ; *Buffer[14997]  0.6
	fadd st0,st1	   ; 0,2 * (*Ascan[2999]) + 0,2 * (*Ascan[2999])
	fstp qword [esi-24]; *Buffer[14996]  0.8

	sub esi,32

       ;FILL loop
       loop_interp_real:

	fld qword [edi-8] ;*Ascan[2998]
	fld qword [edi]   ;*Ascan[2999]
	fst qword [esi]   ;*Buffer[14995]

	fmul st0,st6	  ; *Ascan[2999] *0.2
	fxch st1,st0
	fmul st0,st6	  ; *Ascan[2998] *0.2 -> st0=0.2*2998 st1=0.2*2999 st2=? st3=? st4=? st5=? st6=0.2

	fst st2
	fst st3
	fst st4
	fst st5
	fadd st2,st0
	fadd st3,st0
	fadd st4,st0
	fadd st2,st0
	fadd st3,st0
	fadd st2,st0

	fxch st0,st1

	fadd st2,st0
	fadd st3,st0
	fadd st4,st0
	fadd st5,st0
	fadd st3,st0
	fadd st4,st0
	fadd st5,st0
	fadd st4,st0
	fadd st5,st0
	faddp st5,st0

	fstp st0	    ; st0=1 st1=2 st2=3 st3=4 st4=0.2
	fst qword [esi-32]
	fxch st0,st1
	fst qword [esi-24]
	fxch st0,st2
	fst qword [esi-16]
	fxch st0,st3
	fst qword [esi-8]

	sub esi,40
	sub edi,8

	cmp esi,ebp	    ;exit because *Buffer[0] reached
	jne loop_interp_real

	;beginnig range
	fld qword [edi]     ;*Ascan[0]
	fstp qword [esi]    ;*Buffer[0]

	fstp st0
	fstp st0
	fstp st0
	fstp st0
	fstp st0	   ; FPU stack empty
	;;;;;;;;;;;;;;;;;;;Buffer real filled

      ;ascan complex?
      mov eax,[esp+72]	  ;*AScan_complex
      cmp eax,0
      jne buff_complex1
      jmp fill_real_bild  ; NO Complex AScan

     ;create buffer complex
     buff_complex1:
	mov ecx,[esp+8]   ;Width
	mov ebx,[esp+76]  ;*Buffer_complex
	mov edx,[esp+72]  ;*AScan_complex

	fld qword [edx]   ;AScan_complex[0]
      b0_c:
	add edx,8
	fadd qword [edx]
	dec ecx
	jnz b0_c	  ;AScan[width]

	mov ecx,[esp+8]   ;Width
	fst qword [ebx]   ;buffer[0]

      b1_c:
	add edx,8
	fadd qword [edx]
	add ebx,8
	fst qword [ebx]   ;buffer[1]
	dec ecx
	jnz b1_c


	mov eax,edx	  ; edx ->vorauseilend um width
	mov ecx,[esp+8]   ; width
	shl ecx,4	  ;imul ecx,16       ; 2*width ->addr
	add ecx,8
	sub eax, ecx	  ;hinterher um 2*width +1

	mov ecx,[esp+36]  ;n_AScan
	sub ecx,[esp+8]   ; -width
	sub ecx,[esp+8]   ; -width
	sub ecx,1	  ;n_AScan-2*Width-1

	mov ebp,8
      b2_c:
	fadd qword [edx+ebp] ;vorauseilend
	fsub qword [eax+ebp] ;hinterher
	fst qword [ebx+ebp]  ;buffer[ebx]
	add ebp,8
	sub ecx,1
	jnz b2_c	     ; n_AScan - width

	add edx,ebp
	add eax,ebp
	add ebx,ebp

	mov ecx,[esp+8]   ;width
      b3_c:
	add eax,8	  ;hinterher
	fsub qword [eax]
	add ebx,8
	fst qword [ebx]   ;buffer[ebx]
	dec ecx
	jnz b3_c	  ;n_AScan

	fstp  st0	  ;pop st0
     ;end create xsum buffer_complex

     ;l_interpol_COMPLEX (range 5x)
	mov edi,[esp+76]  ;*Buffer_read[0]    ;;;;;[esp+32] *Ascan[0]
	mov ecx,[esp+36]  ;n_AScan
	dec ecx
	shl ecx,3	  ;double ptr *ascan[2999]
	add edi,ecx	  ;double ptr *ascan[2999]

	mov esi,[esp+76]  ;*Buffer_write[0]
	mov ebp,esi	  ;*Buffer_write[0]

	mov edx,[esp+36]  ;n_AScan
	mov eax,5	  ;interp_range!!!
	mul edx 	  ;eax*edx  result 64bit: [EDX EAX] !!!!
	dec eax
	shl eax,3	  ;double ptr *buffer[14999]
	add esi,eax	  ;double ptr *buffer[14999]

	;prefill FPU Stack
	fld1
	fld1
	fadd st1,st0
	fldz
	fldz
	fldz
	fadd st0,st4  ;st0=2
	fadd st0,st4  ;st0=4
	fadd st0,st0  ;st0=8
	fadd st0,st4  ;st4=2, st3=1; st2=0; st1=0; st0=10

	fdiv st4,st0	 ;0,2 = st4; st3 = ???(buffer), st2 = ???(buffer)  st1 = ???(buffer), st0 = ???(buffer)

	;last 4pixel position
	fld qword [edi]    ;*Ascan[2999]
	fmul st0,st5	   ;0,2 * (*Ascan[2999])
	fst qword [esi]    ; *Buffer[14999]  0.2
	fst st1
	fadd st0,st1
	fst qword [esi-8]  ; *Buffer[14998]  0.4
	fadd st0,st1
	fst qword [esi-16] ; *Buffer[14997]  0.6
	fadd st0,st1	   ; 0,2 * (*Ascan[2999]) + 0,2 * (*Ascan[2999])
	fstp qword [esi-24]; *Buffer[14996]  0.8

	sub esi,32

       ;FILL loop
       loop_interp_complex:

	fld qword [edi-8] ;*Ascan[2998]
	fld qword [edi]   ;*Ascan[2999]
	fst qword [esi]   ;*Buffer[14997]

	fmul st0,st6	  ; *Ascan[2999] *0.2
	fxch st1,st0
	fmul st0,st6	  ; *Ascan[2998] *0.2 -> st0=0.2*2998 st1=0.2*2999 st2=? st3=? st4=? st5=? st6=0.2

	fst st2
	fst st3
	fst st4
	fst st5
	fadd st2,st0
	fadd st3,st0
	fadd st4,st0
	fadd st2,st0
	fadd st3,st0
	fadd st2,st0

	fxch st0,st1

	fadd st2,st0
	fadd st3,st0
	fadd st4,st0
	fadd st5,st0
	fadd st3,st0
	fadd st4,st0
	fadd st5,st0
	fadd st4,st0
	fadd st5,st0
	faddp st5,st0

	fstp st0	; st0=1 st1=2 st2=3 st3=4 st4=0.2
	fst qword [esi-32]
	fxch st0,st1
	fst qword [esi-24]
	fxch st0,st2
	fst qword [esi-16]
	fxch st0,st3
	fst qword [esi-8]

	sub esi,40
	sub edi,8

	cmp esi,ebp	;exit because *Buffer[0] reached
	jne loop_interp_complex

	;beginnig range
	fld qword [edi]     ;*Ascan[0]
	fstp qword [esi]    ;*Buffer[0]

	fstp st0
	fstp st0
	fstp st0
	fstp st0
	fstp st0	   ; FPU stack empty
	;;;;;;;;;;;;;;;;;;;Buffer COMPLEX fill

     jmp fill_real_bild
    ;;;;;end xsum preparation


    ;;;;;;;;;;;;;;;;;;;begin interpol
    interpol:

      ;l_interpol_real (range 5x)
	mov edi,[esp+32]  ;*Ascan[0]
	mov ecx,[esp+36]  ;n_AScan
	dec ecx
	shl ecx,3	  ;double ptr *ascan[2999]
	add edi,ecx	  ;double ptr *ascan[2999]

	mov esi,[esp+40]  ;*Buffer_write[0]
	mov ebp,esi	  ;*Buffer_write[0]

	mov edx,[esp+36]  ;n_AScan
	mov eax,5	  ;interp_range!!!
	mul edx 	  ;eax*edx  result 64bit: [EDX EAX] !!!!
	dec eax
	shl eax,3	  ;double ptr *buffer[14999]
	add esi,eax	  ;double ptr *buffer[14999]

	;prefill FPU Stack
	fld1
	fld1
	fadd st1,st0
	fldz
	fldz
	fldz
	fadd st0,st4  ;st0=2
	fadd st0,st4  ;st0=4
	fadd st0,st0  ;st0=8
	fadd st0,st4  ;st4=2, st3=1; st2=0; st1=0; st0=10

	fdiv st4,st0	 ;0,2 = st4; st3 = ???(buffer), st2 = ???(buffer)  st1 = ???(buffer), st0 = ???(buffer)

	;last 4pixel position
	fld qword [edi]    ;*Ascan[2999]
	fmul st0,st5	   ;0,2 * (*Ascan[2999])
	fst qword [esi]    ; *Buffer[14999]  0.2
	fst st1
	fadd st0,st1
	fst qword [esi-8]  ; *Buffer[14998]  0.4
	fadd st0,st1
	fst qword [esi-16] ; *Buffer[14997]  0.6
	fadd st0,st1	   ; 0,2 * (*Ascan[2999]) + 0,2 * (*Ascan[2999])
	fstp qword [esi-24]; *Buffer[14996]  0.8

	sub esi,32

       ;FILL loop
       loop_interp_only_real:

	fld qword [edi-8] ;*Ascan[2998]
	fld qword [edi]   ;*Ascan[2999]
	fst qword [esi]   ;*Buffer[14997]

	fmul st0,st6	  ; *Ascan[2999] *0.2
	fxch st1,st0
	fmul st0,st6	  ; *Ascan[2998] *0.2 -> st0=0.2*2998 st1=0.2*2999 st2=? st3=? st4=? st5=? st6=0.2

	fst st2
	fst st3
	fst st4
	fst st5
	fadd st2,st0
	fadd st3,st0
	fadd st4,st0
	fadd st2,st0
	fadd st3,st0
	fadd st2,st0

	fxch st0,st1

	fadd st2,st0
	fadd st3,st0
	fadd st4,st0
	fadd st5,st0
	fadd st3,st0
	fadd st4,st0
	fadd st5,st0
	fadd st4,st0
	fadd st5,st0
	faddp st5,st0

	fstp st0	; st0=1 st1=2 st2=3 st3=4 st4=0.2
	fst qword [esi-32]
	fxch st0,st1
	fst qword [esi-24]
	fxch st0,st2
	fst qword [esi-16]
	fxch st0,st3
	fst qword [esi-8]

	sub esi,40
	sub edi,8

	cmp esi,ebp	;exit because *Buffer[0] reached
	jne loop_interp_only_real

	;beginnig range
	fld qword [edi]     ;*Ascan[0]
	fstp qword [esi]    ;*Buffer[0]

	fstp st0
	fstp st0
	fstp st0
	fstp st0
	fstp st0	   ; FPU stack empty
	;;;;;;;;;;;;;;;;;;;Buffer real filled

      ;ascan complex?
      mov eax,[esp+72]	  ;*AScan_complex
      cmp eax,0
      jne buff_complex2
      jmp fill_real_bild  ; NO Complex AScan

     ;l_interpol_COMPLEX (range 5x)
     buff_complex2:
	mov edi,[esp+72]  ;*Ascan_complex_read[0]
	mov ecx,[esp+36]  ;n_AScan
	dec ecx
	shl ecx,3	  ;double ptr *ascan[2999]
	add edi,ecx	  ;double ptr *ascan[2999]

	mov esi,[esp+76]  ;*Buffer_write[0]
	mov ebp,esi	  ;*Buffer_write[0]

	mov edx,[esp+36]  ;n_AScan
	mov eax,5	  ;interp_range!!!
	mul edx 	  ;eax*edx  result 64bit: [EDX EAX] !!!!
	dec eax
	shl eax,3	  ;double ptr *buffer[14999]
	add esi,eax	  ;double ptr *buffer[14999]

	;prefill FPU Stack
	fld1
	fld1
	fadd st1,st0
	fldz
	fldz
	fldz
	fadd st0,st4  ;st0=2
	fadd st0,st4  ;st0=4
	fadd st0,st0  ;st0=8
	fadd st0,st4  ;st4=2, st3=1; st2=0; st1=0; st0=10

	fdiv st4,st0	 ;0,2 = st4; st3 = ???(buffer), st2 = ???(buffer)  st1 = ???(buffer), st0 = ???(buffer)

       ;last 4pixel position
	fld qword [edi]    ;*Ascan[2999]
	fmul st0,st5	   ;0,2 * (*Ascan[2999])
	fst qword [esi]    ; *Buffer[14999]  0.2
	fst st1
	fadd st0,st1
	fst qword [esi-8]  ; *Buffer[14998]  0.4
	fadd st0,st1
	fst qword [esi-16] ; *Buffer[14997]  0.6
	fadd st0,st1	   ; 0,2 * (*Ascan[2999]) + 0,2 * (*Ascan[2999])
	fstp qword [esi-24]; *Buffer[14996]  0.8

	sub esi,32

       ;FILL loop
       loop_interp_only_complex:

	fld qword [edi-8] ;*Ascan[2998]
	fld qword [edi]   ;*Ascan[2999]
	fst qword [esi]   ;*Buffer[14997]

	fmul st0,st6	  ; *Ascan[2999] *0.2
	fxch st1,st0
	fmul st0,st6	  ; *Ascan[2998] *0.2 -> st0=0.2*2998 st1=0.2*2999 st2=? st3=? st4=? st5=? st6=0.2

	fst st2
	fst st3
	fst st4
	fst st5
	fadd st2,st0
	fadd st3,st0
	fadd st4,st0
	fadd st2,st0
	fadd st3,st0
	fadd st2,st0

	fxch st0,st1

	fadd st2,st0
	fadd st3,st0
	fadd st4,st0
	fadd st5,st0
	fadd st3,st0
	fadd st4,st0
	fadd st5,st0
	fadd st4,st0
	fadd st5,st0
	faddp st5,st0

	fstp st0	; st0=1 st1=2 st2=3 st3=4 st4=0.2
	fst qword [esi-32]
	fxch st0,st1
	fst qword [esi-24]
	fxch st0,st2
	fst qword [esi-16]
	fxch st0,st3
	fst qword [esi-8]

	sub esi,40
	sub edi,8

	cmp esi,ebp	;exit because *Buffer[0] reached
	jne  loop_interp_only_complex

	;beginnig range
	fld qword [edi]     ;*Ascan[0]
	fstp qword [esi]    ;*Buffer[0]

	fstp st0
	fstp st0
	fstp st0
	fstp st0
	fstp st0	   ; FPU stack empty
	;;;;;;;;;;;;;;;;;;;Buffer COMPLEX fill


    jmp fill_real_bild
    ;;;;;;;;;;;;;;end interpol





    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;fill images
    ;;;;;;;;;;;;;;;;;;;;;;;;;
    fill_real_bild:

    ;init mm7 with zero
    ;mov eax,0           ;
    ;cvtsi2ss xmm7,eax   ; xmm7 = 0
    ;shufps   xmm7,xmm7,0; 0,0,0,0
    ;CVTPS2PI mm7,xmm7   ; mm7  = 0

    ;pixel_pos
    mov edx,[esp+44]	 ;*pixelpos
    movss  xmm7,[edx+8]  ;pixelpos      0,0,0,Z
    shufps xmm7,xmm7,0	 ;pixelpos      Z,Z,Z,Z
    movlps xmm7,[edx]	 ;pixelpos      Z(TRASH),Z,Y,X

    ;rec_pos
    mov edx,[esp+52]	 ;*receiverpos
    movss  xmm6,[edx+8]  ;receiverpos   0,0,0,Z
    shufps xmm6,xmm6,0	 ;receiverpos   Z,Z,Z,Z
    movlps xmm6,[edx]	 ;receiverpos   Z(TRASH),Z,Y,X

    ;sender_pos
    mov edx,[esp+56]	 ;*senderpos
    movss  xmm5,[edx+8]  ;senderpos     0,0,0,Z
    shufps xmm5,xmm5,0	 ;senderpos     Z,Z,Z,Z
    movlps xmm5,[edx]	 ;senderpos     Z(TRASH),Z,Y,X

    ;resolut
    mov edx,[esp+64]	 ;*resolut
    movss xmm4,[edx]	 ;t,t,t,resolut

    ;factor = 1/(speed*timeinterval)
    mov eax,5		 ; 1 for interp_ratio 1 -> 5 for actual !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    cvtsi2ss xmm3,eax	 ;xmm3=1
    mov edx,[esp+60]	 ;speed
    movss xmm2,[edx]
    mov edx,[esp+68]	 ;timeinterval
    movss xmm1,[edx]
    mulss xmm2,xmm1
    divss xmm3,xmm2	 ;xmm3 = 1/(speed*timeinterval)
    unpcklps xmm4,xmm3	 ; t,t,factor,resolut

    ;interleave in xmm4
    mov eax,-1		 ;4294967295
    cvtsi2ss xmm3,eax	 ;xmm3 = 1

    ;fixed interpol factor
    mov eax,5
    cvtsi2ss xmm1,eax	 ;xmm1 = 5

    mov eax,[esp+36]	 ;n_Ascan
    cvtsi2ss xmm2,eax	 ;xmm2 = 3000

    mulss xmm2,xmm1	 ;3000*5 ->15000
    cvtss2si eax,xmm2;
    mov dword [esp+8],eax ;15000
    unpcklps xmm3,xmm2	 ;xmm3 = t,t,15000,-1
    shufps xmm4,xmm3,68  ;0100 0100b -> xmm4 = 15000,-1,factor,resolut

    mov ecx,[esp+48]	 ; n_pixX
    mov edx,[esp+84]	 ; n_pixY
    mov edi,[esp+88]	 ; n_pixZ

    mov esi,[esp+72]	 ; *ascan_complex

    mov ebp,0		 ; delta IMAGE

   Z_loop:

     Y_loop_init:
     mov edx,[esp+84]	  ; n_pixY
     mov eax,[esp+44]	  ;*pixelpos
     movlps xmm7,[eax]	  ;pixelpos     (t),Z(akt),Y(0),X(0_but_is_notimportant)

     Y_loop:
       ;pix z,y
       movaps xmm3,xmm7     ;          (trash),z_p,y_p,x_p
       shufps xmm3,xmm3,165 ; 10100101b -> z_p,z_p,y_p,y_p
       ;receiver z,y
       movaps xmm2,xmm6     ;     (trash),z_r,y_r,x_r
       shufps xmm2,xmm2,153 ; 10011001b -> (t),(t),z_r,y_r       psrldq xmm2,4        ;      SSE2 shift right by BYTES not BITS! t,t,z,y
       ;sender  z,y
       movaps xmm1,xmm5     ;     (trash),z_s,y_s,x_s
       shufps xmm1,xmm1,153 ; 10011001b -> (t),(t),z_s,y_s       psrldq xmm1,4        ;      SSE2 shift right by BYTES not BITS! t,t,z,y
       ;interleave
       unpcklps xmm2,xmm1   ;     z_s,z_r,y_s,y_r

       subps xmm3,xmm2	    ;     pix-senderpos pix-receiverpos
       mulps xmm3,xmm3	    ;     quadrieren

       movaps xmm2,xmm3     ;     z_s,z_r,y_s,y_r
       shufps xmm2,xmm2,238 ;11101110b-> t,t,z_s,z_r            psrldq xmm2,8        ;SSE2 shift right by BYTES not BITS! t,t,z_s,z_r
       addps  xmm3,xmm2     ;     (t,t,z_s,z_r) + (t,t,y_s,y_r)  -> t,t,S,R
       unpcklps xmm3,xmm3   ;     S,S,R,R  ;!!!!shufps xmm3,xmm3,68  ;     0100 0100b -> S,R,S,R    ;

       X_loop_init:
	 mov ecx,[esp+48]     ;    n_pixX
	 mov eax,[esp+44]     ;    *pixelpos
	 movss xmm2,[eax]     ;    pixelpos[1] = X(0)
	 movss xmm7,xmm2      ;    t,Z(akt),Y(akt),X(0)

       X_loop:		     ;     !!!2 PIXEL  parallel!!!
	 ;x_p1 & p_x0
	 movss	xmm2,xmm7     ;    pixel  t,t,t,x_p
	 shufps xmm2,xmm2,0   ;    x_p,x_p,x_p,x_p
	 addss	xmm2,xmm4     ;    x_p,x_p,x_p,x_p + t,t,t,resolut -> x_p0,x_p0,x_p0,x_p1
	 shufps xmm2,xmm2,17  ;    0001 0001b ->  x_p1,x_p0,x_p1,x_p0

	 ;x_r &  x_s
	 movss xmm1,xmm6      ;    receiver (trash),z_r,y_r,x_r
	 shufps xmm1,xmm5,0   ;    sender; 0000 0000b -> S,S,R,R

	 subps xmm2,xmm1      ;    x_p-x_r, x_p-x_s -> S1,S0,R1,R0
	 mulps xmm2,xmm2      ;    quadrieren

	 addps xmm2,xmm3      ;    S1x,S0x,R1x,R0x + Syz,Syz,Ryz,Ryz
	 sqrtps xmm2,xmm2     ;    sqrt

	 movaps xmm1,xmm2     ;
	 shufps xmm1,xmm1,254 ;    1111 1110b -> S1,S1,S1,S0
	 addps	xmm2,xmm1     ;    S1,S0,R1,R0 +  S1,S1,S1,S0 = t,t,P1,P0

	 ;lauflaenge in t zu index
	 movaps xmm1,xmm4
	 shufps xmm1,xmm1,85  ;    0101 0101b -> factor,factor,factor,factor
	 mulps xmm2,xmm1      ;    t*factor=index

	 ;time to index
	 cvtss2si eax,xmm2    ;    index_value

	;;;;;;;;;;;;;;;;;;;;;;;;
	;rangecheck pixel 1
	 cmp eax,dword [esp+8]	 ;n_Ascan *5      ;cmp eax,dword [esp+36] ;n_Ascan
	 jge outrange_xsum1	 ;0....2999
	 cmp eax,0		 ;
	 jl outrange_xsum1

	 ;inrange real
	 shl eax,3		;imul eax,8    ;index-> 64bit (double) addr   NP
	 mov ebx,[esp+40]	;*buffer_real[index_aktual]               NP
	 fld qword [ebx+eax]	;fpu double move

	 mov ebx,[esp+100]	;*image_MAP
	 shr ebp,1		; half delta for SINGLE
	 fld dword [ebx+ebp]	; load single
	 fmulp st1,st0		; mul pixel value with map value; pop stack
	 shl ebp,1		; double delta for DOUBLE

	 mov ebx,[esp+92]	;*image_sum_real
	 fld qword [ebx+ebp]
	 faddp st1,st0		;image_sum_real + (actual_value*map)

	 mov ebx,[esp+28]	;*bild_real
	 fstp qword [ebx+ebp]	; ebp = *bild_real[index_aktual]

	   ;complex Ascan ?
	   cmp esi,0		 ;dword [esp+72],0  ;*AScan_complex
	   jz rangecheck_pixel_2

	   mov ebx,[esp+76]	 ;*buffer_complex[index_aktual]
	   fld qword [ebx+eax]	 ;fpu move

	   mov ebx,[esp+100]	  ;*image_MAP
	   shr ebp,1		  ; half delta for SINGLE
	   fld dword [ebx+ebp]	  ; load single
	   fmulp st1,st0	  ; mul pixel value with map value; pop stack
	   shl ebp,1		  ; double delta for DOUBLE

	   mov ebx,[esp+96]	 ;*image_sum_compl
	   fld qword [ebx+ebp]
	   faddp st1,st0

	   mov ebx,[esp+80]	 ;*bild_compl[index_aktual]
	   fstp qword [ebx+ebp]  ; ebp = *bild_compl[index_aktual]
	   jmp rangecheck_pixel_2

	outrange_xsum1:
	 ;real
	 mov ebx,[esp+92]	 ;*image_sum_real
	 fld qword [ebx+ebp]
	 mov ebx,[esp+28]	 ;*bild_real
	 fstp qword [ebx+ebp]	 ; ebp = *bild_real[index_aktual]

	   ;Complex AScan ?
	   cmp esi,0		 ;dword [esp+72],0  ;*AScan_complex
	   jz rangecheck_pixel_2 ;jump out if only real

	   mov ebx,[esp+96]	 ;*image_sum_compl
	   fld qword [ebx+ebp]

	   mov ebx,[esp+80]	 ;*bild_compl[index_aktual]
	   fstp qword [ebx+ebp]  ; ebp = *bild_compl[index_aktual]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;
	rangecheck_pixel_2:

	 shufps xmm2,xmm2,85	; 0101 0101b -> P1,P1,P1,P1
	 cvtss2si eax,xmm2	; index_value

	 cmp eax,dword [esp+8]	;n_Ascan *5   ;cmp eax,[esp+36]    ;n_Ascan
	 jge outrange_xsum2	;0....2999
	 cmp eax,0		;
	 jl outrange_xsum2

	 ;inrange real
	 shl eax,3		;imul eax,8    ;index-> 64bit (double) addr   NP
	 mov ebx,[esp+40]	;*buffer_real[index_aktual]               NP
	 fld qword [ebx+eax]	;fpu double move

	 mov ebx,[esp+100]	;*image_MAP
	 shr ebp,1		; half delta for SINGLE
	 fld dword [ebx+ebp+4]	; load single
	 fmulp st1,st0		; mul pixel value with map value; pop stack
	 shl ebp,1		; double delta for DOUBLE

	 mov ebx,[esp+92]	;*image_sum_real
	 fld qword [ebx+ebp+8]	; delta Image +1pixel
	 faddp st1,st0

	 mov ebx,[esp+28]	;*bild_real
	 fstp qword [ebx+ebp+8] ; ebp = *bild_real[index_aktual]

	   ;complex Ascan ?
	   cmp esi,0		 ;dword [esp+72],0  ;*AScan_complex
	   jz out_pixel2

	   mov ebx,[esp+76]	 ;*buffer_complex[index_aktual]
	   fld qword [ebx+eax]	 ;fpu move

	   mov ebx,[esp+100]	  ;*image_MAP
	   shr ebp,1		  ; half delta for SINGLE
	   fld dword [ebx+ebp+4]  ; load single
	   fmulp st1,st0	  ; mul pixel value with map value; pop stack
	   shl ebp,1		  ; double delta for DOUBLE

	   mov ebx,[esp+96]	  ;*image_sum_compl
	   fld qword [ebx+ebp+8]  ; delta Image +1pixel
	   faddp st1,st0

	   mov ebx,[esp+80]	  ;*bild_compl[index_aktual]
	   fstp qword [ebx+ebp+8] ; ebp = *bild_compl[index_aktual]
	   jmp out_pixel2

	outrange_xsum2:
	 ;real
	 mov ebx,[esp+92]	;*image_sum_real
	 fld qword [ebx+ebp+8]
	 mov ebx,[esp+28]	;*bild_real
	 fstp qword [ebx+ebp+8] ; ebp = *bild_real[index_aktual]

	   ;Complex AScan ?
	   cmp esi,0		 ;dword [esp+72],0  ;*AScan_complex
	   jz out_pixel2	 ;jump out if only real

	   mov ebx,[esp+96]	  ;*image_sum_compl
	   fld qword [ebx+ebp+8]

	   mov ebx,[esp+80]	 ;*bild_compl[index_aktual]
	   fstp qword [ebx+ebp+8]  ; ebp = *bild_compl[index_aktual]

	;;;;;;;;;;;
	out_pixel2:

	;add x_p + 2pixel
	addss xmm7,xmm4  ; t,t,t,x_p  + t,t,t,resolut
	addss xmm7,xmm4  ; t,t,t,x_p  + t,t,t,resolut

	;inc image index (2pixel)
	add ebp,16	 ;*bild_real[index_aktual] +2pixel[double]

	sub ecx,2
	cmp ecx,2
	jge X_loop

	;odd x size???
	cmp ecx,1
	jne exit_X_loop  ;even
	  ;odd x size
	  add ecx,1
	  subss xmm7,xmm4      ; t,t,t,x_p  + t,t,t,resolut
	  sub ebp,8	       ;*bild_real[index_aktual] +2pixel[double]
	  jmp  X_loop

	exit_X_loop:
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

      ;add y_p + 1pixel
      xorps xmm1,xmm1	   ; xmm1 = 0
      movss xmm1,xmm4	   ; t,t,t,resolut
      shufps xmm1,xmm1,81  ; 01010001b -> 0,0,resolut,0
      addps xmm7,xmm1	   ; t,t,y_p,t  + 0,0,resolut,0

      sub edx,1
      jnz Y_loop
      ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ;add z_p + 1pixel
    xorps xmm1,xmm1	   ; xmm0 = 0
    movss xmm1,xmm4	   ; t,t,t,resolut
    shufps xmm1,xmm1,69    ; 01000101b -> 0,resolut,0,0
    addps xmm7,xmm1	   ; t,z_p,t,t  + 0,resolut,0,0

    sub edi,1
    jnz Z_loop
    jmp exit
    ;;;;end sum_branch;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;






    ;;;;;;;;;;;;;;;;;;;;;;;;;;
    exit:
	;emms             ;reset mmx, ready fpu
	;FLDCW [esp+4]    ;restore old CTRWORD            ;buffering in stack (16bit!) + FWAIT to ensure save is complete (FSTCW without wait)


	;mov eax,[esp+8] ;out width (previous test)

	pop eax
	pop eax

	pop ebx
	pop edi
	pop esi
	pop ebp

      retn


 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,

	;FNINIT und FLDCW
	;MOVD MM0,eax ; edx nach MM0
	;MOVD MM1,eax ; eax nach MM1
	;PSLLQ MM0,32 ; MM0 um 32 Bit schieben
	;POR MM0,MM1 ; MM1 dazu 'ORen'



      ;inrange:   NOT 2 pipe version
	;sub eax,1          ;1 addr-> 0 addr                         U
	;shl eax,3          ;imul eax,8    ;index-> 64bit addr       U
	;movd ebx,mm1       ;*buffer
	;add eax,ebx        ;*buffer[index]

	;64 bit mmx variante
	;movq mm0, qword [eax]       ;                          U
	;MOVntq qword [edx], mm0     ; writethrough             U
				     ; MMX raised keine flags (I hope)

	;add edx,8      ; + 1 double addr
	;paddd mm2,mm4   ; mm2 + 4  = + 1 int32 addr
	;sub ecx,1
	;jnz fill
	;jmp exit
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	;32-bit variante
	;mov ebx,dword [eax]     ;sum[ecx]<-buffer[index]
	;mov dword [edx],ebx
	;mov ebx,dword [eax+4]   ;sum[ecx]<-buffer[index]
	;mov dword [edx+4],ebx

      ;64bit mmx memcopy variante
       ;movq mm0, qword [eax]     ;movq nur upipe(?)           U
       ;MOVntq qword [edx], mm0 ;writethrough moveq