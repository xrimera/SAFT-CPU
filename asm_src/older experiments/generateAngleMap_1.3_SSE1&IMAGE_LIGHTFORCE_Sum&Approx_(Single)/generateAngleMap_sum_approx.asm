[segment .text align=16]
global	_gam					  ; UNIX
global gam					  ; Windows

;void as2v(double* bild, double* AScan,int n_AScan, double* buffer, double* index_vol, int n_index, double* rec_pos,double* em2pix, double *speed, double* resolution,double* time intervall,double* AScan_complex, double* buffer_complex, double* bild)+68

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; version 1.3 approx+lightforce map
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;
;old ;                 new                      newer
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;28=*bild                                     -> *bild
;32=*Ascan                                    -> *map
;36=n_Ascan                                   -> n_map
;40=*buffer                                   -> *rec_pos
;44=*pixel_index      ->  *pixel_vect[1 1 1]  -> *rec_norm
;48=n_index           ->  n_pixX              -> *send_pos
;52=*receiverpos                              -> *send_norm
;56=*dist_em_pixel    ->  *senderpos          -> *pix_vect
;60=*speed                                    -> n_X
;64=*resolut                                  -> n_Y
;68=*time_int                                 -> n_Z
;72=*Ascan_complex                            -> *res
;76=*buff_complex                             -> *LF_sum
;80=*bild_compl                               -> *LF_out
;84=                  ->  n_pixY
;88=                  ->  n_pixZ
;92=                  ->  *IMAGE_SUM
;96=                  ->  *IMAGE_SUM_compl

gam:						  ; UNIX
_gam:						  ; Windows

	push ebp
	push esi
	push edi
	push ebx	 ;+12 (diff:esp+8)

	push eax	 ;buffer for width  +8 (diff esp+12)
	push eax	 ;buffer for CTW    +4 (diff esp+16)

	fninit		 ;init fpu


    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;fill images

    ;xmm7 pixel_pos
    mov edx,[esp+56]	 ;*pixelpos
    movss  xmm7,[edx+8]  ;pixelpos      0,0,0,Z
    shufps xmm7,xmm7,64  ;0100 0000b -> pixelpos 0,Z,Z,Z
    movlps xmm7,[edx]	 ;pixelpos      0(TRASH),Z,Y,X

    ;xmm6 receiver & sender X
    ;mov ebx,[esp+40]     ; *receiverpos
    ;movss xmm6,[ebx]
    ;mov ebx,[esp+48]     ; *senderpos
    ;movss xmm5,[ebx]
    ;UNPCKLPS xmm6,xmm5   ; 0,0,X_s,X_r
    ;shufps xmm6,xmm6,    ; 0100 0100b -> X_s,X_r,X_s,X_r

    ;xmm6 rec_pos
    ;mov edx,[esp+40]     ;*receiverpos
    ;movss  xmm6,[edx+8]  ;receiverpos   0,0,0,Z
    ;shufps xmm6,xmm6,64  ;receiverpos   0,Z,Z,Z
    ;movlps xmm6,[edx]    ;receiverpos   0(TRASH),Z,Y,X

    ;xmm5 rec_norm
    ;mov edx,[esp+44]     ;*receiverpos
    ;movss  xmm5,[edx+8]  ;receiverpos   0,0,0,Z
    ;shufps xmm5,xmm5,64  ;receiverpos   0,Z,Z,Z
    ;movlps xmm5,[edx]    ;receiverpos   0(TRASH),Z,Y,X

    ;xmm4 sender_pos
    ;mov    edx,[esp+48]  ;*senderpos
    ;movss  xmm4,[edx+8]  ;senderpos     0,0,0,Z
    ;shufps xmm4,xmm4,64  ;senderpos     0,Z,Z,Z
    ;movlps xmm4,[edx]    ;senderpos     0(TRASH),Z,Y,X

    ;xmm3 sender_norm
    ;mov    edx,[esp+52]  ;*senderpos
    ;movss  xmm3,[edx+8]  ;senderpos     0,0,0,Z
    ;shufps xmm3,xmm3,64  ;senderpos     0,Z,Z,Z
    ;movlps xmm3,[edx]    ;senderpos     0(TRASH),Z,Y,X

    ;xmm7 map-mul-factor  & length 1-3000 -> 0-2999
    mov ebx,1
    cvtsi2ss xmm0,ebx
    cvtsi2ss xmm1,[esp+36]; length of map
    subss xmm1,xmm0	  ; 3000-1
    cvtss2si ebx,xmm1	  ; 2999 to int32
    mov [esp+36],ebx	  ; [esp+36] <- 2999 !!!!!!
    shufps xmm1,xmm7,160  ; 1010 0000b -> Z,Z,2999,2999
    shufps xmm7,xmm1,52   ; 0011 0100b -> mul_fact,Z,Y,X

    ;xmm3 resolut
    ;mov edx,[esp+72]     ;*resolut
    ;movss xmm2,[edx]     ;t,t,t,resolut
    ;shufps xmm2,xmm3,160 ;1010 0000b -> z,z,res,res
    ;shufps xmm3,xmm2,52  ;0011 0100b -> res,Z,Y,X

    ;define gpr
    mov ecx,[esp+60]	 ; n_pixX
    mov edx,[esp+64]	 ; n_pixY
    mov edi,[esp+68]	 ; n_pixZ

    mov esi,[esp+28]	 ; *bild

    mov ebp,0		 ; delta IMAGE

   Z_loop:

     Y_loop_init:
     mov edx,[esp+64]	  ; n_pixY
     mov eax,[esp+56]	  ; *pixelpos
     movlps xmm7,[eax]	  ; pixelpos     (t),Z(akt),Y(0),X(0_but_is_notimportant)

     Y_loop:
     movaps xmm6,xmm7
     shufps xmm6,xmm6,165 ; 1010 0101b -> Z_p,Z_p,Y_p,Y_p

     mov ebx,[esp+40]	  ; *receiverpos
     movlps xmm5,[ebx+4]  ; t,t,Z_r,Y_r
     mov eax,[esp+48]	  ; *senderpos
     movlps xmm4,[eax+4]  ; t,t,Z_s,Y_s
     UNPCKLPS xmm5,xmm4   ; Z_s,Z_r,Y_s,Y_r
     subps xmm6,xmm5	  ; Z_ps,Z_pr,Y_ps,Y_pr  (pix_pos-send_pos)  XMM6!!!

     movaps xmm5,xmm6
     mulps xmm5,xmm5	  ; Zҟps,Zҟpr,Yҟps,Yҟpr
     movaps xmm4,xmm5
     shufps xmm4,xmm4,78  ; 0100 1110b ->Yҟps,Yҟpr,Zҟps,Zҟpr
     addps xmm5,xmm4	  ; YҫZҟps,YҫZҟpr,YҫZҟps,YҫZҟpr      XMM5!!!!

       X_loop_init:
	 mov ecx,[esp+60]     ; n_pixX
	 mov eax,[esp+56]     ; *pixelpos
	 movss xmm0,[eax]
	 movss xmm7,xmm0      ; t,Z(akt),Y(akt),X(0)

       X_loop:		      ;
	 ; X_s & X_r
	 mov ebx,[esp+40]     ; *receiverpos
	 mov eax,[esp+48]     ; *senderpos
	 movss xmm3,[ebx]
	 movss xmm2,[eax]
	 UNPCKLPS xmm3,xmm2   ; 0,0,X_s,X_r
	 shufps xmm3,xmm3,68  ; 0100 0100b -> X_s,X_r,X_s,X_r

	 ; X P1 & P0
	 movss xmm4,xmm7      ; X_p[akt]
	 shufps xmm4,xmm4,0   ; x_p,x_p,x_p,x_p
	 mov ebx,[esp+72]     ; *resolut
	 addss xmm4,[ebx]     ; x_p,x_p,x_p,x_p1
	 shufps xmm4,xmm4,5   ; 0000 0101b ->  x_p1,x_p1,x_p0,x_p0

	 ;sub X_p - (X_r & X_s)
	 subps xmm4,xmm3      ; (x_p1,x_p1,x_p0,x_p0)-(X_s,X_r,X_s,X_r) -> X_s1,X_r1,X_s0,X_r0  XMM4!!!!

	 ;vabs
	 movaps xmm3,xmm4     ;
	 mulps xmm3,xmm3      ; Xҟsp1,Xҟrp1,Xҟsp0,Xҟrp0,
	 addps xmm3,xmm5      ; 2pixel & P_S & P_R Length!
	 ;sqrtps xmm3,xmm3     ; -> L_s1,L_r1,L_s0,L_r0                                XMM3!!!!
	 rsqrtps xmm3,xmm3    ; -> L_s1,L_r1,L_s0,L_r0     !!!WArning APPROX. RECIP. SQRT!!!!    XMM3!!!!

	 ; Pixel 0+1 (X)
	 ;divps xmm4,xmm3      ; normed  X_s1,X_r1,X_s0,X_r0
	 mulps xmm4,xmm3      ; normed  X_s1,X_r1,X_s0,X_r0    !!!WArning APPROX. RECIP. SQRT!!!!    XMM3!!!!

	 ; Pixel 0+1 (Y)
	 movaps xmm2,xmm6
	 shufps xmm2,xmm2,68  ; 0100 0100b -> Y_ps,Y_pr,Y_ps,Y_pr
	 ;divps xmm2,xmm3      ; normed  Y_s1,Y_r1,Y_s0,Y_r0
	 mulps xmm2,xmm3      ; normed  Y_s1,Y_r1,Y_s0,Y_r0    !!!WArning APPROX. RECIP. SQRT!!!!    XMM3!!!!

	 ; Pixel 0+1 (Z)
	 movaps xmm1,xmm6
	 shufps xmm1,xmm1,238 ; 1110 1110b -> Z_ps,Z_pr,Z_ps,Z_pr
	 ;divps xmm1,xmm3      ; normed  Z_s1,Z_r1,Z_s0,Z_r0
	 mulps xmm1,xmm3      ; normed  Z_s1,Z_r1,Z_s0,Z_r0   !!!WArning APPROX. RECIP. SQRT!!!!    XMM3!!!!

	 ;vDot mul
	 mov ebx,[esp+44]
	 movss xmm0,[ebx]    ; rec_norm X
	 mov eax,[esp+52]
	 movss xmm3,[eax]    ; send_norm X
	 unpcklps xmm0,xmm3  ;
	 shufps xmm0,xmm0,68 ;0100 0100b -> X_s,X_r,X_s,X_r
	 mulps xmm4,xmm0

	 movss xmm0,[ebx+4]  ; rec_norm Y
	 movss xmm3,[eax+4]  ; send_norm Y
	 unpcklps xmm0,xmm3  ;
	 shufps xmm0,xmm0,68 ;0100 0100b -> Y_s,Y_r,Y_s,Y_r
	 mulps xmm2,xmm0

	 movss xmm0,[ebx+8]  ; rec_norm Z
	 movss xmm3,[eax+8]  ; send_norm Z
	 unpcklps xmm0,xmm3  ;
	 shufps xmm0,xmm0,68 ;0100 0100b -> Z_s,Z_r,Z_s,Z_r
	 mulps xmm1,xmm0

	 ;VDot add
	 addps xmm4,xmm2
	 addps xmm4,xmm1     ; VDot_S1,VDot_R1,VDot_S0,VDot_R0

	 ;VDot_R * VDot_S  (2pixel)
	 movaps xmm3,xmm4
	 shufps xmm3,xmm3,177; 1011 0001b -> VDot_R1,VDot_S1,VDot_R0,VDot_S0
	 mulps xmm4,xmm3     ; P1,P1,P0,P0

	 ;0....1 to Map(0...end)
	 movaps xmm2,xmm7
	 shufps xmm2,xmm2,255 ; 1111 1111b -> mul_fact,mul_fact,mul_fact,mul_fact
	 mulps	xmm4,xmm2     ; 2999* 0...1

	 ;SSE boundary check 2999
	 ;cmpps  xmm2,xmm4,5   ; le 2999 -> inverse -> nlt 2999
	 ;andps  xmm4,xmm2     ; set to 0 if >2999
	 ;SSE boundary check 0
	 ;xorps  xmm3,xmm3
	 ;cmpps  xmm3,xmm4,2   ; nlt 0   -> inverse -> le 0
	 ;andps  xmm4,xmm3     ; set to 0 if <0

	 ;P0 -> get map value write to bild
	 cvtss2si eax,xmm4	  ; index_value P0      !!!!introduced TRUNCATION ROUNDING ERROR (0.5) FOR INCREASING "CORRECT" ROUNDING AREA  FROM 0.5...2999.5 TO 0.999...2999.999!!!!
	   ;integer boundary check
	   xor ebx,ebx
	   cmp eax,[esp+36]
	   cmovg eax,[esp+36]
	   cmp eax,ebx
	   cmovl eax,ebx
	 shl eax,2		  ; imul eax,4    ;index-> 32bit (single) addr
	 mov ebx,dword [esp+32]   ; *Map
	 movss xmm0,[ebx+eax]	  ; SSE LOAD

	 ;P1 -> get map value write to bild
	 shufps xmm4,xmm4,170	  ; 1010 1010b -> P1
	 cvtss2si eax,xmm4	  ; index_value P1
	   ;integer boundary check
	   xor ebx,ebx
	   cmp eax,[esp+36]
	   cmovg eax,[esp+36]
	   cmp eax,ebx
	   cmovl eax,ebx
	 shl eax,2		  ; imul eax,4    ;index-> 32bit (single) addr
	 mov ebx,dword [esp+32]   ; *Map
	 movss xmm1,[ebx+eax]	  ; SSE LOAD

	 ;;;write P0 & P1         ; SSE 32bit Temporal move
	 movss [esi+ebp],xmm0
	 movss [esi+ebp+4],xmm1   ;

	 ;;;write LIGHTFORCE_SUM
	 mov ebx,[esp+76]	  ; *old_sum
	 mov eax,[esp+80]	  ; *new_sum

	 addss xmm0,[ebx+ebp]
	 addss xmm1,[ebx+ebp+4]

	 movss [eax+ebp],xmm0
	 movss [eax+ebp+4],xmm1       ;


	 ;;;;MMX NONTEMPORAL MOVE
	 ;psllq mm1,32             ; to upper 32bit
	 ;por mm0,mm1              ; join Pixel0 (mm0) & pixel1 (mm1)

	 ;;;write P0 & P1           ; MMX NON-TEMPORAL MOVE
	 ;movntq [esi+ebp],mm0     ; ebp = *bild_real[index_aktual]




	;;;;;;;;;;;
	out_pixel1:

	;add x_p + 2pixel
	mov ebx,[esp+72]	  ;*resolut
	addss xmm7,[ebx]	  ; t,t,t,x_p  + t,t,t,resolut
	addss xmm7,[ebx]

	;inc image index +2pixel
	add ebp,8		  ;*bild_real[index_aktual] +2pixel[single]

	sub ecx,2
	cmp ecx,2
	jge X_loop
	      ;odd x size???
	      cmp ecx,1
	      jne exit_X_loop  ;even = 0
		;odd x size = 1
		mov ebx,[esp+72]     ;*resolut
		subss xmm7,[ebx]     ; t,t,t,x_p  - t,t,t,resolut
		add ecx,1
		sub ebp,4	     ;*bild_real[index_aktual] -1pixel[single]
		jmp  X_loop

	exit_X_loop:
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

      ;add y_p + 1pixel
      mov ebx,[esp+72]	   ;*resolut
      movss xmm0,[ebx]	   ; 0,0,0,resolu
      shufps xmm0,xmm0,81  ; 0101 0001b -> 0,0,resol,0
      addps xmm7,xmm0	   ; t,t,y_p,t  + 0,0,resolut,0

      sub edx,1
      jnz Y_loop
      ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ;add z_p + 1pixel
    mov ebx,[esp+72]	 ;*resolut
    movss xmm0,[ebx]	 ; 0,0,0,resolu
    shufps xmm0,xmm0,69  ; 0100 0101b -> 0,resol,0,0
    addps xmm7,xmm0	 ; t,z_p,t,t  + 0,resolut,0,0

    sub edi,1
    jnz Z_loop
    jmp exit
    ;;;;end sum_branch;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



    ;;;;;;;;;;;;;;;;;;;;;;;;;;
    exit:
	sfence
	emms		  ;reset mmx, ready fpu
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