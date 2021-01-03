# DX11SkinningWithFBX

![gif](https://media.giphy.com/media/LLz2zsyX47XXLf7cEf/giphy.gif)

- FBX를 읽어오는 코드를 사용하여 버텍스 버퍼(위치, 노말, UV), 인덱스 버퍼를 생성 및 복사. 
- FBX를 읽어오는 코드를 사용하여 스켈레톤의 구조 및 바인드 포즈 정보와 테이크 별 스켈레탈 애니메이션을 위한 변환 정보를 불러옴.
- DX11ObjectWithFBX 에서는 간단하게 버퍼 두개를 사용하여 렌더링 했다면, 여기서는 처음에 정점 정보를 SRV로 참조하여 computeshader에서 계산하여 UAV로 저장하고, 이를 CopyResource 를 사용하여 버텍스 버퍼로 복사하여 렌더링함. 기존의 쉐이더 코드는 그대로 유지.
	※ UAV와 버텍스 버퍼 동시에 사용이 안되어 이렇게 세팅
- 애니메이션은 저장되어 있던 하나의 테이크를 처음부터 끝까지 computeShader에서 보여줌.
- 파이프라인 의존성 데이터 분류를 dx12 처럼 copy, compute, draw 로 나눔.

# TODO

- 데이터 로딩 굉장히 느림.
- TRS <-> Affine <-> DQ 계산 코드 추가 필요
- dqs, normal approximation, CRS 등 여러 방법 미구현
