FROM python

RUN apt-get update -y
RUN apt-get install apt-utils git autoconf zlib1g-dev tcl tk tcl8.6-doc build-essential -y

RUN git clone https://github.com/GamesCrafters/GamesmanClassic.git
WORKDIR GamesmanClassic

RUN autoconf && ./configure && make

ENV DISPLAY=host.docker.internal:0

RUN cd bin && ./XGamesman.new


