#Useing a lightweight python base image
FROM python:3.11-slim

#Set the working directory inside the container
WORKDIR /app

#Copy the requirements file first( this caches the installation step to speed up future builds)
COPY requirements.txt .

#Install the python dependencies...
RUN pip install --no-cache-dir -r requirements.txt

#Copy your python script into the container
COPY logger.py .

#Create an empty CSV file so the volume mount doesn't get confused later
RUN touch attendence.csv

#Tell docker what command to run when the container starts
CMD ["python", "logger.py"]

