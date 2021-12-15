FROM python:3.9.9-slim-buster

RUN pip install \
    pytest \
    numpy
