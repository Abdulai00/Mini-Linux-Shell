FROM alpine:latest

# Install build dependencies, including readline for the d8sh build
RUN apk add --no-cache gcc musl-dev make readline-dev

# Set the working directory
WORKDIR /app

# Copy the source code to the container
COPY . .

# Build the project using make
RUN make

# Check that the d8sh binary is created and has executable permissions
RUN ls -l /app
RUN chmod +x /app/d8sh

# Set the command to run the d8sh shell
CMD ["./d8sh"]

# command to run: docker run -it d8sh_app

