blockchain.out: *.cpp *.h
	g++ *.cpp -o blockchain
run:	blockchain
	./blockchain
