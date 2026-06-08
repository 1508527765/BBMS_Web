FROM python:3.8.6

VOLUME /project

WORKDIR /project

RUN apt-get update && apt-get install -y git tzdata

COPY requirements.txt /project/

RUN pip install -r requirements.txt -i https://pypi.tuna.tsinghua.edu.cn/simple

RUN pip install gunicorn

ENV TZ=Asia/Shanghai

COPY . .

EXPOSE 8080

ENTRYPOINT ["gunicorn", "app:app", "-c", "gunicorn.conf.py"]
