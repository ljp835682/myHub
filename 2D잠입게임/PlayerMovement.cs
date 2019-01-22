using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerMovement : MonoBehaviour {

    public Transform startPos;                  //시작지점
    public GameObject evidence;                 //자신의 흔적
    private CharacterController controller;     //캐릭터컨트롤러 컴포넌트

    void Start () {
        transform.position = startPos.position;
        controller = GetComponent<CharacterController>();
    }
	
	//이동
	void Update () {
        float h = Input.GetAxis("Horizontal");
        float v = Input.GetAxis("Vertical");

        controller.Move(new Vector3(h * 0.1f, 0, v * 0.1f));
    }
}
