# Use an official C++ image as a parent image
FROM gcc:latest

# Install required packages: clang-format and build-essential for Makefile and compiling C++
RUN apt-get update && apt-get install -y \
    clang-format \
    make \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory in the container
WORKDIR /usr/src/app

# Specify the default command as shell to allow interactive or manual actions
CMD ["/bin/bash"]