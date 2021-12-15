FROM python:3.7.12-slim-buster

RUN pip install \
    pytest \
    numpy
