[segment .text align=16]
global	_as2v_complex


;void as2v(double* bild, double* AScan,int n_AScan, double* buffer, double* index_vol, int n_index, double* rec_pos,double* em2pix, double *speed, double* resolution,double* time intervall,double* AScan_complex, double* buffer_complex, double* bild)+68
;
_as2v_complex:

	push ebp
	push esi
	push edi
	push ebx	 ;+12 (diff:esp+8)

	push eax	 ;buffer for width  +8 (diff esp+12)
	push eax	 ;buffer for CTW    +4 (diff esp+16)

	fninit		 ;init fpu

     ;calc width
	FNSTCW [esp+4]	 ;buffering in stack (16bit!) + FWAIT to ensure save is complete (FSTCW without wait)
	mov ax,[esp+4]
	and ax,0F3FFh	 ;delete 2 bits
	or  ax,0000100000000000b ;round up +inf
	mov [esp+6],ax	 ;new CTRWORD
	FLDCW [esp+6]


	fldz		     ;load zero
	fld1		     ;load 1
	mov eax,[esp+68]     ;time intervall
	fld qword [eax]
	mov eax,[esp+64]     ;resolution
	fld qword [eax]
	mov eax,[esp+60]     ;speed
	fld qword [eax]

	fdivp st1,st0	     ; res/speed; destination/source ,pop
	fdivrp st1,st0	     ; ergeb/timeinterv; source/dest ,pop -> width now in st0

	fcom st0,st1	     ; zb: 0.4 cmp 1  -> lower
	fcmovb st0,st2	     ; transfer st2 to st0 if below -> width<1 -> interpol not sum!

	mov [esp+8],dword 2
	fidiv dword [esp+8]  ; halbieren
	fistp dword [esp+8]  ; width to int32 (ceiling round)

	FLDCW [esp+4]	     ;restore old CTRWORD
	fstp	st0
	fstp	st0	     ; -> fpu stack emtpy now
	;end calc width
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

     ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



      ;width to big?
	mov ecx,[esp+8]   ;Width

	mov eax,[esp+36]  ;n_Ascan
	shr eax,1	  ;n_aScan/2
	sub eax,2
	cmp ecx,eax
	jl xs_int_check   ;width < (n_ascan/2)-2
	jmp exit

      ;xsum or interpol?
      xs_int_check:
	cmp ecx,0	  ;width 0 check
	jne xsum
	jmp interpol
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



     ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
     ;;;;Xsum path
     xsum:
	mov ebx,[esp+40]  ;*Buffer
	mov edx,[esp+32]  ;*AScan

	fld qword [edx]   ;AScan[0]
      b0:
	add edx,8
	fadd qword [edx]
	dec ecx
	jnz b0		  ;AScan[width]

	mov ecx,[esp+8]   ;Width
	fst qword [ebx]   ;buffer[0]

      b1:
	add edx,8
	fadd qword [edx]
	add ebx,8
	fst qword [ebx]   ;buffer[1]
	dec ecx
	jnz b1


	mov eax,edx	  ; edx ->vorauseilend um width
	mov ecx,[esp+8]   ; width
	shl ecx,4	  ;imul ecx,16       ; 2*width ->addr
	add ecx,8
	sub eax, ecx	  ;hinterher um 2*width +1

	mov ecx,[esp+36]  ;n_AScan
	sub ecx,[esp+8]   ; -width
	sub ecx,[esp+8]   ; -width
	sub ecx,1	  ;n_AScan-2*Width-1

      b2:
	fadd qword [edx+8]
	add edx,8	   ;vorauseilend
	fsub qword [eax+8]
	add eax,8	   ;hinterher
	fst qword [ebx+8]  ;buffer[ebx]
	add ebx,8
	sub ecx,1
	jnz b2		   ; n_AScan - width

	mov ecx,[esp+8]   ;width
      b3:
	add eax,8	  ;hinterher
	fsub qword [eax]
	add ebx,8
	fst qword [ebx]   ;buffer[ebx]
	dec ecx
	jnz b3		  ;n_AScan

	fstp  st0	  ;pop st0
     ;end create xsum buffer_real


      mov eax,[esp+72]	  ;*AScan_complex
      cmp eax,0
      jne buff_complex
      jmp fill_real_bild  ; NO Complex  AScan

     ;create buffer complex
     buff_complex:
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

      b2_c:
	fadd qword [edx+8]
	add edx,8	   ;vorauseilend
	fsub qword [eax+8]
	add eax,8	   ;hinterher
	fst qword [ebx+8]  ;buffer[ebx]
	add ebx,8
	sub ecx,1
	jnz b2_c	   ; n_AScan - width

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

    ;;;;;;;;;;;;;;;;;;;;;;;;;
    fill_real_bild:

       ;fill fpu stack
       fld1		   ; fld 1

       mov edx,[esp+68]    ;*timeintervall
       fld qword [edx]	   ; time_intervall

       mov ebx,[esp+60]    ;*speed
       fld qword [ebx]	   ; speed

       fmulp st1,st0
       fdivp st1,st0	   ; 1 / timeint*speed

       mov eax,[esp+52]    ;*receiver_pos
       fld qword [eax]	   ; receiver_pos_x
       fld qword [eax+8]   ; receiver_pos_y
       fld qword [eax+16]  ; receiver_pos_z

       mov ecx,[esp+48]    ;n_index
       mov edx,[esp+28]    ;*bild
       mov edi,[esp+80]    ;*bild_complex

       mov esi,[esp+72]    ;*AScan_complex (zero if ONLY real)
       mov ebp,0	   ; DELTA Double pointer

   wdh_xsum:
       mov eax,[esp+44]    ;*index  3xN double
       fld qword [eax]	   ; pixel_pos_x
       fld qword [eax+8]   ; pixel_pos_y
       fld qword [eax+16]  ; pixel_pos_z

       fsub st0,st3	   ; pixel_pos_z - receiver_posz
       fxch st0,st1
       fsub st0,st4	   ; pixel_pos_y - receiver_posy
       fxch st0,st2
       fsub st0,st5	   ; pixel_pos_x - receiver_posx

       fmul st0,st0
       fxch st0,st1
       fmul st0,st0
       fxch st0,st2
       fmul st0,st0
       faddp st1,st0
       faddp st1,st0
       fsqrt

       mov ebx,[esp+56] 	 ;dist_em_pixel
       fld qword [ebx+ebp]
       faddp st1,st0		 ;gesamt_dist
       fmul st0,st4
       fistp dword [esp+8]	 ;index_value 0....xxxx (width_buffer overidden)
       mov eax,[esp+8]		 ;index_value

     ;;;;;;;;;;;;;;;;;;;;;;;;
     ;rangecheck
       cmp eax,[esp+36]   ;n_Ascan
       jge outrange_xsum  ;0....2999
       cmp eax,0	  ;
       jl outrange_xsum

     ;;;;;;;;;;;;;;;;
     ;inrange real
       shl eax,3	  ;imul eax,8    ;index-> 64bit (double) addr   NP
       mov ebx,[esp+40]   ;*buffer_real[index_aktual]               NP

       ;fpu version
       fld qword [ebx+eax]
       fstp qword [edx+ebp]
       ;32-bit variante
       ;mov ebx,dword [eax]     ;bild[ecx]<-buffer_real[index]
       ;mov dword [edx],ebx
       ;mov ebx,dword [eax+4]   ;bild[ecx]<-buffer_real[index]
       ;mov dword [edx+4],ebx

       ;AScan Complex?
       cmp esi,0		;dword [esp+72],0  ;*AScan_complex
       jz ir_xsum_real

       mov ebx, [esp+76]   ;*buffer_complex[index_aktual]

       ;fpu version
       fld qword [ebx+eax]
       fstp qword [edi+ebp]

     ir_xsum_real:

       ;add [esp+56],dword 8   ;*dist_em_pixel[]
       add [esp+44],dword 24  ;*pixelind[]
       add ebp,8
       ;add edi,8
       ;add edx,8              ;edx+8 = +1 double addr     V
       sub ecx,1	      ;                               V
       jnz wdh_xsum
       jmp exit 	      ;exit xsum path

     ;;;;;;;;;;;;;;;;outrange
     outrange_xsum:
     ;real
       fldz
       fstp qword [edx+ebp]

       ;AScan Complex?
       cmp esi,0		;dword [esp+72],0  ;*AScan_complex
       jz outr_xsum_real

     ;complex
       fldz
       fstp qword [edi+ebp]

       ;mmx version
       ;pxor mm0,mm0              ; mm0 = 0               U
       ;MOVntq qword [edx], mm0   ; writethrough          U
       ;32-bit variante
       ;mov dword [edx],0      ;bild[ecx]<-buffer[index]
       ;mov dword [edx+4],0

      outr_xsum_real:	       ;only real
       ;add [esp+56],dword 8   ;dist_em_pixel
       add [esp+44],dword 24  ;pixelind
       add ebp,8
       ;add edi,8
       ;add edx,8              ;edx+8 = +1 double addr     V
       sub ecx,1
       jnz wdh_xsum	      ;exit xsum path
       jmp exit
      ;;;end xsum path
      ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



     ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
     ;;;;Interpol path
     interpol:

       FLDCW [esp+6]	   ;load controlword (round up +inf)

       fld1		   ; fld 1

       mov edx,[esp+68]    ;*timeintervall
       fld qword [edx]	   ; time_intervall

       mov ebx,[esp+60]    ;*speed
       fld qword [ebx]	   ; speed

       fmulp st1,st0
       fdivp st1,st0	   ; 1 / timeint*speed
			   ; st0 = mul_factor

       mov eax,[esp+52]    ;*receiver_pos
       fld qword [eax]	   ; receiver_pos_x
       fld qword [eax+8]   ; receiver_pos_y
       fld qword [eax+16]  ; receiver_pos_z

       mov ecx,[esp+48]    ;n_index
       mov edx,[esp+28]    ;*bild
       mov edi,[esp+80]    ;*bild_complex

       mov esi,[esp+72]    ;*AScan_complex (zero if ONLY real)
       mov ebp,0	   ; DELTA Double pointer

   wdh_interpol:
       mov eax,[esp+44]    ;*index  3xN double
       fld qword [eax]	   ; pixel_pos_x
       fld qword [eax+8]   ; pixel_pos_y
       fld qword [eax+16]  ; pixel_pos_z

       fsub st0,st3	   ; pixel_pos_z - receiver_posz
       fxch st0,st1
       fsub st0,st4	   ; pixel_pos_y - receiver_posy
       fxch st0,st2
       fsub st0,st5	   ; pixel_pos_x - receiver_posx

       fmul st0,st0
       fxch st0,st1
       fmul st0,st0
       fxch st0,st2
       fmul st0,st0
       faddp st1,st0
       faddp st1,st0
       fsqrt

       mov ebx,[esp+56] 	 ;dist_em_pixel
       fld qword [ebx+ebp]
       faddp st1,st0		 ;gesamt_dist
       fmul st0,st4		 ;index double

       fist dword [esp+8]	 ;NOT POP!
				 ; index_value 0....xxxx (width_buffer overidden)
				 ; round CEILING!
       mov eax,[esp+8]		 ;index_value

     ;rangecheck
       cmp eax,[esp+36]       ;n_Ascan
       jl check2 ;ge outrange_interpol  ;0....2999
       jmp outrange_interpol
      check2:
       cmp eax,1	      ;not 0 because access index-1
       jl outrange_interpol


     ;inrange_interpol:
       fld1		  ;load 1
       fxch st0,st1	  ;st0 = index (double) st1=1  st2-4=receiverpos st5=mul_faktor
       fprem		  ;st0/st1 -> remainder in st0   z.b.: 2.1 -> 0.1
       fsub st1,st0	  ;st0=remainder st1= 1-remainder

      ;real
       shl eax,3	      ;imul eax,8    ;index-> 64bit addr   NP
       mov ebx,[esp+32]       ;*ascan_real[index_aktual]               NP

       fld qword [ebx+eax]    ; value_real ceil
       fld qword [ebx+eax-8]  ; value_real before

       fmul st0,st3	      ;value_before * rem
       fxch st0,st1
       fmul st0,st2	      ;value ceil * 1-rem
       fadd st0,st1	      ;interpol_value st0

       ;move to real_bild
       fstp qword [edx+ebp]   ;move interpol value to bild_REAL!
       fstp st0 	      ; values real popped

       ;check if complex
       cmp esi,0	      ;dword [esp+72],0  ;*AScan_complex
       jz ir_interp_real

     ;complex                 ;EAX filled with deltaADDR from REAL
       mov ebx,[esp+72]       ;*ascan_complex[index_aktual]               NP

       fld qword [ebx+eax]    ; value_complex ceil
       fld qword [ebx+eax-8]  ; value_complex before

       fmul st0,st2	      ;value before * rem
       fxch st0,st1
       fmul st0,st3	      ;value ceil * 1-rem
       fadd st0,st1	      ;interpol value st0

       ;move to complex_bild
       fstp qword [edi+ebp]   ;move interpol value to bild_COMPLEX!
       fstp st0


     ir_interp_real:

       fstp st0
       fstp st0 	      ;only load: st0-2=receiverpos st3=mulfactor

       ;add [esp+56],dword 8  ;dist_em_pixel
       add [esp+44],dword 24  ;pixelind
       ;add edx,8             ;edx+8 = +1 double addr     V
       add ebp,8
       sub ecx,1	      ;                               V
       jnz wdh_interpol
       jmp exit 	      ;exit xsum path


     outrange_interpol:
      ;real part
       ;FPU zer0 load variante
       fldz
       fstp qword [edx+ebp]	;add 0 to bild_real

       ;AScan Complex?
       cmp esi,0		;dword [esp+72],0  ;*AScan_complex
       jz outr_interp_real

     ;complex
       fldz
       fstp qword [edi+ebp]	;add 0 to bild_complex

       ;32-bit variante
       ;mov dword [eax],0       ;bild_complex[ecx]<-buffer_complex[index]
       ;mov dword [eax+4],0
       ;MMX ver
       ;pxor mm0,mm0             ; mm0 = 0               U
       ;MOVntq qword [edx], mm0  ; writethrough          U

      outr_interp_real:
       fstp st0 		; only result to be popped

       ;add [esp+56],dword 8    ;dist_em_pixel
       add [esp+44],dword 24	;pixelind
       ;add edx,8               ;edx+8 = +1 double addr     V
       add ebp,8
       sub ecx,1
       jnz wdh_interpol 	       ;exit xsum path
       jmp exit
      ;;;end xsum path
      ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;




    ;;;;;;;;;;;;;;;;;;;;;;;;;;
    exit:
	;FNINIT und FLDCW
	;MOVD MM0,eax ; edx nach MM0
	;MOVD MM1,eax ; eax nach MM1
	;PSLLQ MM0,32 ; MM0 um 32 Bit schieben
	;POR MM0,MM1 ; MM1 dazu 'ORen'

	FLDCW [esp+4]	 ;restore old CTRWORD            ;buffering in stack (16bit!) + FWAIT to ensure save is complete (FSTCW without wait)
	emms		 ;reset mmx, ready fpu

	;mov eax,[esp+8] ;out width



	pop eax
	pop eax

	pop ebx
	pop edi
	pop esi
	pop ebp

      retn


 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,
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
       ;MOVntq qword [edx], mm0 ;writethrough moveq          U          ;MMX raised no flags(?); nur upipe(?)