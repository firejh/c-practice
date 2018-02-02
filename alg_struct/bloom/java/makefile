all:
	make clean
	@mkdir -p bin/
	javac -d bin/ antispam/bloom.java
	javac -d bin/ antispam/converter.java
	javac -d bin/ antispam/antispam.java
	@cd bin/ && java antispam/converter ../tl.txt ../tl.dict
	@cd bin/ && java antispam/antispam ../tl.txt ../tl.dict

clean:
	rm -rf ./bin/
	rm -f ./tl.dict
