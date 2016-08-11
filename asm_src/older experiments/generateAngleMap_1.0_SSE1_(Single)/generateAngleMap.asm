[segment .text align=16]
global	_gam


;void as2v(double* bild, double* AScan,int n_AScan, double* buffer, double* index_vol, int n_index, double* rec_pos,double* em2pix, double *speed, double* resolution,double* time intervall,double* AScan_complex, double* buffer_complex, double* bild)+68

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
;76=*buff_complex
;80=*bild_compl
;84=                  ->  n_pixY
;88=                  ->  n_pixZ
;92=                  ->  *IMAGE_SUM
;96=                  ->  *IMAGE_SUM_compl

_gam:

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

    ;xmm6 rec_pos
    mov edx,[esp+40]	 ;*receiverpos
    movss  xmm6,[edx+8]  ;receiverpos   0,0,0,Z
    shufps xmm6,xmm6,64  ;receiverpos   0,Z,Z,Z
    movlps xmm6,[edx]	 ;receiverpos   0(TRASH),Z,Y,X

    ;xmm5 rec_norm
    mov edx,[esp+44]	 ;*receiverpos
    movss  xmm5,[edx+8]  ;receiverpos   0,0,0,Z
    shufps xmm5,xmm5,64  ;receiverpos   0,Z,Z,Z
    movlps xmm5,[edx]	 ;receiverpos   0(TRASH),Z,Y,X

    ;xmm4 sender_pos
    mov    edx,[esp+48]  ;*senderpos
    movss  xmm4,[edx+8]  ;senderpos     0,0,0,Z
    shufps xmm4,xmm4,64  ;senderpos     0,Z,Z,Z
    movlps xmm4,[edx]	 ;senderpos     0(TRASH),Z,Y,X

    ;xmm3 sender_norm
    mov    edx,[esp+52]  ;*senderpos
    movss  xmm3,[edx+8]  ;senderpos     0,0,0,Z
    shufps xmm3,xmm3,64  ;senderpos     0,Z,Z,Z
    movlps xmm3,[edx]	 ;senderpos     0(TRASH),Z,Y,X

    ;xmm5 ZERO
     ; xmm5 -> 0,t,t,t

    ;xmm4 map-mul-factor
    mov ebx,1
    cvtsi2ss xmm0,ebx
    cvtsi2ss xmm1,dword [esp+36] ;length of map
    subss xmm1,xmm0	 ; 3000-1
    shufps xmm1,xmm4,160 ;1010 0000b -> z,z,res,res
    shufps xmm4,xmm1,52  ;0011 0100b -> mul_fact,Z,Y,X

    ;xmm3 resolut
    mov edx,[esp+72]	 ;*resolut
    movss xmm2,[edx]	 ;t,t,t,resolut
    shufps xmm2,xmm3,160 ;1010 0000b -> z,z,res,res
    shufps xmm3,xmm2,52  ;0011 0100b -> res,Z,Y,X

    ;define gpr
    mov ecx,[esp+60]	 ; n_pixX
    mov edx,[esp+64]	 ; n_pixY
    mov edi,[esp+68]	 ; n_pixZ

    mov esi,[esp+28]	 ; *bild

    mov ebp,0		 ; delta IMAGE

   Z_loop:

     Y_loop_init:
     mov edx,[esp+64]	  ; n_pixY
     mov eax,[esp+56]	  ;*pixelpos
     movlps xmm7,[eax]	  ;pixelpos     (t),Z(akt),Y(0),X(0_but_is_notimportant)

     Y_loop:
       X_loop_init:
	 mov ecx,[esp+60]     ;    n_pixX
	 mov eax,[esp+56]     ;    *pixelpos
	 movss xmm2,[eax]
	 movss xmm7,xmm2      ;    t,Z(akt),Y(akt),X(0)

       X_loop:		      ;

	;;Pixel 0
	;;;sender - pixel
	 movaps xmm2,xmm7
	 subps	xmm2,xmm4     ; pix_pos -send_pos
	 movaps xmm1,xmm2

	 ;vabs
	 mulps	xmm1,xmm1     ; t²,z²,y²,x²
	 movaps xmm0,xmm1
	 shufps xmm0,xmm0,165 ; 1010 0101b ->   z,z,y,y
	 addss	xmm1,xmm0     ; x+y
	 shufps xmm0,xmm0,170 ; 1010 1010b ->   z,z,z,z
	 addss	xmm1,xmm0     ; (x+y)+z
	 sqrtss xmm1,xmm1     ; sqrt(z²+y²+x²) -> length of (pix_pos-send_pos)
	 shufps xmm1,xmm1,0   ;
	 divps	xmm2,xmm1     ; (pix_pos-send_pos) / (sqrt(z²+y²+x²)) -> normalized vector now

	 ;vdot
	 mulps	xmm2,xmm3     ; v * send_norm
	 movaps xmm1,xmm2
	 shufps xmm1,xmm1,165 ; 1010 0101b ->   z,z,y,y
	 addss	xmm2,xmm1     ; x+y
	 shufps xmm1,xmm1,170 ; 1010 1010b ->   z,z,z,z
	 addss	xmm2,xmm1     ; (x+y)+z

	 movss [esp+8],xmm2 ; zwischen speichern in bild

       ;;;receiver - pixel
	 movaps xmm2,xmm7
	 subps	xmm2,xmm6     ; pix_pos -rec_pos
	 movaps xmm1,xmm2

	 ;vabs
	 mulps	xmm1,xmm1     ; t²,z²,y²,x²
	 movaps xmm0,xmm1
	 shufps xmm0,xmm0,165 ; 1010 0101b ->   z,z,y,y
	 addss	xmm1,xmm0     ; x+y
	 shufps xmm0,xmm0,170 ; 1010 1010b ->   z,z,z,z
	 addss	xmm1,xmm0     ; (x+y)+z
	 sqrtss xmm1,xmm1     ; sqrt(z²+y²+x²) -> length of (pix_pos-send_pos)
	 shufps xmm1,xmm1,0   ;
	 divps	xmm2,xmm1     ; (pix_pos-rec_pos) / (sqrt(z²+y²+x²)) -> normalized vector now

	 ;vdot
	 mulps	xmm2,xmm5     ; v * rec_norm
	 movaps xmm1,xmm2
	 shufps xmm1,xmm1,165 ; 1010 0101b ->   z,z,y,y
	 addss	xmm2,xmm1     ; x+y
	 shufps xmm1,xmm1,170 ; 1010 1010b ->   z,z,z,z
	 addss	xmm2,xmm1     ; (x+y)+z
	 mulss	xmm2,[esp+8]; send_pix * rec_pix

	 ;0....1 to Map(0...end)
	 movaps xmm1,xmm4
	 shufps xmm1,xmm1,255 ; 1111 1111b -> mul_fact,mul_fact,mul_fact,mul_fact
	 mulss	xmm2,xmm1     ; 2999* 0...1
	 cvtss2si eax,xmm2    ; index_value

	 ;get map value write to bild
	 shl eax,2		  ;imul eax,4    ;index-> 32bit (single) addr
	 mov  ebx,dword [esp+32]  ;*Map
	 movd mm0,[ebx+eax]	  ;mmx dword move
	 ;movss [esi+ebp],xmm2     ; ebp = *bild_real[index_aktual]

	 ;;;;;;;;out pixel 0:
	 ;add x_p + 1pixel
	 movaps xmm1,xmm3
	 shufps xmm1,xmm1,255	;1111 1111b -> res,res,res,res
	 addss	xmm7,xmm1	; t,t,t,x_p  + t,t,t,resolut

      ;;;;;Pixel 1
	;;;sender - pixel
	 movaps xmm2,xmm7
	 subps	xmm2,xmm4     ; pix_pos-send_pos
	 movaps xmm1,xmm2

	 ;vabs
	 mulps	xmm1,xmm1     ; t²,z²,y²,x²
	 movaps xmm0,xmm1
	 shufps xmm0,xmm0,165 ; 1010 0101b ->   z,z,y,y
	 addss	xmm1,xmm0     ; x+y
	 shufps xmm0,xmm0,170 ; 1010 1010b ->   z,z,z,z
	 addss	xmm1,xmm0     ; (x+y)+z
	 sqrtss xmm1,xmm1     ; sqrt(z²+y²+x²) -> length of (pix_pos-send_pos)
	 shufps xmm1,xmm1,0   ;
	 divps	xmm2,xmm1     ; (pix_pos-send_pos) / (sqrt(z²+y²+x²)) -> normalized vector now

	 ;vdot
	 mulps	xmm2,xmm3     ; v * send_norm
	 movaps xmm1,xmm2
	 shufps xmm1,xmm1,165 ; 1010 0101b ->   z,z,y,y
	 addss	xmm2,xmm1     ; x+y
	 shufps xmm1,xmm1,170 ; 1010 1010b ->   z,z,z,z
	 addss	xmm2,xmm1     ; (x+y)+z

	 movss [esp+8],xmm2 ; zwischen speichern in bild

       ;;;receiver - pixel
	 movaps xmm2,xmm7
	 subps	xmm2,xmm6     ; pix_pos -rec_pos
	 movaps xmm1,xmm2

	 ;vabs
	 mulps	xmm1,xmm1     ; t²,z²,y²,x²
	 movaps xmm0,xmm1
	 shufps xmm0,xmm0,165 ; 1010 0101b ->   z,z,y,y
	 addss	xmm1,xmm0     ; x+y
	 shufps xmm0,xmm0,170 ; 1010 1010b ->   z,z,z,z
	 addss	xmm1,xmm0     ; (x+y)+z
	 sqrtss xmm1,xmm1     ; sqrt(z²+y²+x²) -> length of (pix_pos-send_pos)
	 shufps xmm1,xmm1,0   ;
	 divps	xmm2,xmm1     ; (pix_pos-rec_pos) / (sqrt(z²+y²+x²)) -> normalized vector now

	 ;vdot
	 mulps	xmm2,xmm5     ; v * rec_norm
	 movaps xmm1,xmm2
	 shufps xmm1,xmm1,165 ; 1010 0101b ->   z,z,y,y
	 addss	xmm2,xmm1     ; x+y
	 shufps xmm1,xmm1,170 ; 1010 1010b ->   z,z,z,z
	 addss	xmm2,xmm1     ; (x+y)+z
	 mulss	xmm2,[esp+8]; send_pix * rec_pix

	 ;0....1 to Map(0...end)
	 movaps xmm1,xmm4
	 shufps xmm1,xmm1,255 ; 1111 1111b -> mul_fact,mul_fact,mul_fact,mul_fact
	 mulss	xmm2,xmm1     ; 2999* 0...1
	 cvtss2si eax,xmm2    ; index_value

	 ;get map value write to bild
	 shl eax,2		  ;imul eax,4    ;index-> 32bit (single) addr
	 mov ebx,dword [esp+32]   ;*Map
	 movd mm1,[ebx+eax]	  ;mmx dword move
	 psllq mm1,32		  ;to upper 32bit
	 por mm0,mm1		  ;join Pixel0 (mm0) & pixel1 (mm1)
	 movntq [esi+ebp],mm0	  ;MMX NON TEMPORAL MOVE ebp = *bild_real[index_aktual]


	;;;;;;;;;;;
	out_pixel1:

	;add x_p + 1pixel
	movaps xmm1,xmm3
	shufps xmm1,xmm1,255   ;1111 1111b -> res,res,res,res
	addss xmm7,xmm1        ; t,t,t,x_p  + t,t,t,resolut

	;inc image index
	add ebp,8	       ;*bild_real[index_aktual] +2pixel[single]

	sub ecx,2
	cmp ecx,2
	jge X_loop
	      ;odd x size???
	      cmp ecx,1
	      jne exit_X_loop  ;even = 0
		;odd x size = 1
		inc ecx
		movaps xmm1,xmm3
		shufps xmm1,xmm1,255 ;1111 1111b -> res,res,res,res
		subss xmm7,xmm1      ; t,t,t,x_p  + t,t,t,resolut
		sub ebp,4	     ;*bild_real[index_aktual] +2pixel[single]
		jmp  X_loop

	exit_X_loop:
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

      ;add y_p + 1pixel
      movaps xmm2,xmm5	   ; 0,t,t,t
      shufps xmm2,xmm2,255 ; 0,0,0,0
      movaps xmm1,xmm3	   ; resolu,t,t,t
      shufps xmm1,xmm2,15  ; 0000 1111b -> 0,0,resol,resol
      shufps xmm1,xmm1,243 ; 1111 0011b -> 0,0,resolut,0
      addps xmm7,xmm1	   ; t,t,y_p,t  + 0,0,resolut,0

      sub edx,1
      jnz Y_loop
      ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ;add z_p + 1pixel
    movaps xmm2,xmm5	 ; 0,t,t,t
    shufps xmm2,xmm2,255 ; 0,0,0,0
    movaps xmm1,xmm3	 ; resolu,t,t,t
    shufps xmm2,xmm1,240 ; 1111 0000b -> (t,resol),resol,0,0
    addps xmm7,xmm2	 ; t,z_p,t,t  + (t!!!),resolut,0,0

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