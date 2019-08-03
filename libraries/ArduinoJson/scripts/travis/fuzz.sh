#!/bin/bash -eux

ROOT_DIR=$(dirname $0)/../../
<<<<<<< HEAD
INCLUDE_DIR=$ROOT_DIR/src/
FUZZING_DIR=$ROOT_DIR/fuzzing/
JSON_CORPUS_DIR=$FUZZING_DIR/my_corpus
JSON_SEED_CORPUS_DIR=$FUZZING_DIR/seed_corpus

CXX="clang++-$CLANG"
CXXFLAGS="-g -fprofile-instr-generate -fcoverage-mapping -fsanitize=address,fuzzer"

$CXX $CXXFLAGS -o json_fuzzer -I$INCLUDE_DIR $FUZZING_DIR/fuzzer.cpp

export ASAN_OPTIONS="detect_leaks=0"
export LLVM_PROFILE_FILE="json_fuzzer.profraw"
./json_fuzzer "$JSON_CORPUS_DIR" "$JSON_SEED_CORPUS_DIR" -max_total_time=60

llvm-profdata-$CLANG merge -sparse json_fuzzer.profraw -o json_fuzzer.profdata

llvm-cov-$CLANG report ./json_fuzzer -instr-profile=json_fuzzer.profdata
=======
INCLUDE_DIR=${ROOT_DIR}/src/
FUZZING_DIR=${ROOT_DIR}/fuzzing/
CXXFLAGS="-g -fprofile-instr-generate -fcoverage-mapping -fsanitize=address,undefined,fuzzer -fno-sanitize-recover=all"

fuzz() {
	NAME="$1"
	FUZZER="${NAME}_fuzzer"
	FUZZER_CPP="${FUZZING_DIR}/${NAME}_fuzzer.cpp"
	CORPUS_DIR="${FUZZING_DIR}/${NAME}_corpus"
	SEED_CORPUS_DIR="${FUZZING_DIR}/${NAME}_seed_corpus"

	clang++-${CLANG} ${CXXFLAGS} -o ${FUZZER} -I$INCLUDE_DIR ${FUZZER_CPP}

	export ASAN_OPTIONS="detect_leaks=0"
	export LLVM_PROFILE_FILE="${FUZZER}.profraw"
	./${FUZZER} "$CORPUS_DIR" "$SEED_CORPUS_DIR" -max_total_time=30 -timeout=1

	llvm-profdata-${CLANG} merge -sparse ${LLVM_PROFILE_FILE} -o ${FUZZER}.profdata
	llvm-cov-${CLANG} report ./${FUZZER} -instr-profile=${FUZZER}.profdata
}

fuzz json
fuzz msgpack
>>>>>>> 71a09487d18c8b01d889cc54dde62335db4d16d6
