FROM ubuntu:latest
COPY adarch /adarch

RUN apt update
RUN apt install -y build-essential python3.5 python3.5-dev
RUN chmod +x /adarch/adarch

CMD ["/bin/bash"]
