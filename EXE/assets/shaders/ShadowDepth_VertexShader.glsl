#version 420

in vec4 vPosition;
in vec4 vBoneID;
in vec4 vBoneWeight;

const int MaxBones = 100;
uniform mat4 matBonesArray[MaxBones];
uniform bool isSkinnedMesh;
uniform mat4 lightSpaceMatrix;
uniform mat4 matModel;

void main()
{
	if (isSkinnedMesh)
	{
		mat4 BoneTransform = matBonesArray[ int(vBoneID[0]) ] * vBoneWeight[0];
		     BoneTransform += matBonesArray[ int(vBoneID[1]) ] * vBoneWeight[1];
		     BoneTransform += matBonesArray[ int(vBoneID[2]) ] * vBoneWeight[2];
		     BoneTransform += matBonesArray[ int(vBoneID[3]) ] * vBoneWeight[3];
		vec4 vertAfterBoneTransform = BoneTransform * vPosition;
		gl_Position = lightSpaceMatrix * matModel * vertAfterBoneTransform;
	}
	else
	{
		gl_Position = lightSpaceMatrix * matModel * vPosition;
	}
    
}