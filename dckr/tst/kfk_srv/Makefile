TAG = 1.0.0
PFX = ikucan/pykafarr_kfk_tst_srvr

build:
	docker build -t $(PFX):$(TAG) .

push: build
	docker push $(PFX):$(TAG)

run: 
	docker run --rm $(PFX):$(TAG)
