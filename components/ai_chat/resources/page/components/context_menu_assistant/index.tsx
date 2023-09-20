/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import * as React from 'react'
import * as ReactDOM from 'react-dom'
import ButtonMenu from '@brave/leo/react/buttonMenu'
import Icon from '@brave/leo/react/icon'
import { getLocale } from '$web-common/locale'
import { showAlert } from '@brave/leo/react/alertCenter'

import getPageHandlerInstance from '../../api/page_handler'
import styles from './style.module.scss'
import FeedbackForm from '../feedback_form'

interface ContextMenuAssistantProps {
  turnText: string
  chatId: number
}

function ContextMenuAssistant_(
  props: ContextMenuAssistantProps,
  ref: React.RefObject<Map<number, Element>>
) {
  const [feedbackId, setFeedbackId] = React.useState<string | null>()
  const [isFormVisible, setIsFormVisible] = React.useState(false)

  const formContainerElement = ref.current?.get(props.chatId)

  const handleCopyText = () => {
    navigator.clipboard.writeText(props.turnText)
  }

  const handleLikeAnswer = () => {
    getPageHandlerInstance()
      .pageHandler.likeMessage(true, props.turnText)
      .then(() => {
        showAlert({
          mode: 'simple',
          type: 'info',
          content: getLocale('answerLiked'),
          actions: []
        })
      })
  }

  const handleDislikeAnswer = () => {
    getPageHandlerInstance()
      .pageHandler.likeMessage(false, props.turnText)
      .then((resp) => {
        setFeedbackId(resp.id)
        showAlert({
          mode: 'simple',
          type: 'info',
          content: getLocale('answerDisliked'),
          actions: [
            {
              text: getLocale('addFeedbackButtonLabel'),
              kind: 'plain',
              action: () => setIsFormVisible(true)
            }
          ]
        })
      })
  }

  const handleFormCancelClick = () => {
    setIsFormVisible(false)
  }

  const handleOnSubmit = (selectedCategory: string, feedbackText: string) => {
    if (feedbackId) {
      getPageHandlerInstance()
        .pageHandler.sendFeedback(selectedCategory, feedbackText, feedbackId)
        .then((resp) => {
          if (resp.isSuccess) {
            showAlert({
              mode: 'simple',
              type: 'success',
              content: getLocale('feedbackSent'),
              actions: []
            })
          }
        })
      setIsFormVisible(false)
    }
  }

  return (
    <>
      <ButtonMenu className={styles.moreButton}>
        <div slot='anchor-content'>
          <Icon name='more-vertical' />
        </div>
        <leo-menu-item onClick={handleCopyText}>
          <Icon name='copy' />
          <span>{getLocale('copyButtonLabel')}</span>
        </leo-menu-item>
        <leo-menu-item onClick={handleLikeAnswer}>
          <Icon name='thumb-up' />
          <span>{getLocale('likeAnswerButtonLabel')}</span>
        </leo-menu-item>
        <leo-menu-item onClick={handleDislikeAnswer}>
          <Icon name='thumb-up' />
          <span>{getLocale('dislikeAnswerButtonLabel')}</span>
        </leo-menu-item>
      </ButtonMenu>
      {formContainerElement &&
        isFormVisible &&
        ReactDOM.createPortal(
          <FeedbackForm
            onSubmit={handleOnSubmit}
            onCancel={handleFormCancelClick}
            isDisabled={!Boolean(feedbackId).valueOf()}
          />,
          formContainerElement
        )}
    </>
  )
}

const ContextMenuAssistant = React.forwardRef(ContextMenuAssistant_)

export default ContextMenuAssistant
