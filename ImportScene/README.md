# ImportScene

기존의 FBXSDK 에서 지원하는 ImportScene 예제를 참조하여 directX11에 맞도록 데이터를 다시 가공한다. 특이점은 FBXSDK에서 제공하는 Triangulate 를 사용하여 모든 메쉬의 프리미티브를 전부 삼각형으로 바꾼다.

- 정점 당 데이터 : 위치, 노말, 탄젠트, 바이노말, UV(여러개)
- 정점 인덱스
- (스켈레톤) : hierarchy 와 연동
- (애니메이션) : 스켈레톤과 연결
