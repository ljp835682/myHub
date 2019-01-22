using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class HpBarTrans : MonoBehaviour {

    public GameObject hpbar;
    public Transform pos;

    // Use this for initialization
    void Start () {
	}
	
	// Update is called once per frame
	void Update () {
        Vector2 vector = Camera.main.WorldToScreenPoint(pos.position);
        hpbar.transform.position = vector;
        float height = Screen.height;
        float width = Screen.width;
        hpbar.GetComponent<RectTransform>().sizeDelta = new Vector2(Mathf.Lerp(70,100, (width * height) /(1920*1080)),20);
    }
}
