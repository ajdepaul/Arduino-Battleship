.file "battleshiptable.S"
.data

; tables storing 12 ships' data
tablex:
	.byte 100,100,100,100,100,100,100,100,100,100,100,100
tabley:
	.byte 100,100,100,100,100,100,100,100,100,100,100,100
tablehit:
	.byte 0,0,0,0,0,0,0,0,0,0,0,0

.global writeShip
.global readShipX
.global readShipY
.global readShipHit

.text

; write ship data
; r24=index, r22=x, r20=y, r18=hit
writeShip:
	; set x value
	ldi r30,lo8(tablex)
	ldi r31,hi8(tablex)
	add r30,24
	adc r31,r1
	st z, r22

	; set y value
	ldi r30,lo8(tabley)
	ldi r31,hi8(tabley)

	add r30,24
	adc r31,r1
	st z, r20

	; set x value
	ldi r30,lo8(tablehit)
	ldi r31,hi8(tablehit)

	add r30,24
	adc r31,r1
	st z, r18
	ret

; retrun ship X
readShipX:
	ldi r30,lo8(tablex)
	ldi r31,hi8(tablex)

	add r30,r24
	adc r31,r1

	ld r24,z
	ret

; return ship Y
readShipY:
	ldi r30,lo8(tabley)
	ldi r31,hi8(tabley)

	add r30,r24
	adc r31,r1

	ld r24,z
	ret

; return ship hit
readShipHit:
	ldi r30,lo8(tablehit)
	ldi r31,hi8(tablehit)

	add r30,r24
	adc r31,r1

	ld r24,z
	ret