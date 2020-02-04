FROM python

RUN apt-get update -y
RUN apt-get install git build-essential autoconf zlib1g-dev tcl-dev tk-dev -y

RUN git clone https://github.com/GamesCrafters/GamesmanClassic.git
WORKDIR GamesmanClassic

RUN autoconf && ./configure && make

CMD cd bin && ./XGamesman.new
