TAG = 1.0.0
PFX = ikucan/pykafarr_build

build:
	docker build -t $(PFX):$(TAG) .

push: build
	docker push $(PFX):$(TAG)

run: 
	docker run --rm $(PFX):$(TAG)
